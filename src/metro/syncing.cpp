namespace metro {
    void DualTarget::add_target(const OID& target, bool wip) {
        if (wip) {
            head = target;
            hasWip = true;
        } else {
            base = target;
            // head == base only if no WIP has been set yet.
            if (head.isNull) {
                head = target;
            }
        }
    }

    bool DualTarget::is_valid(const Repository& repo, map<OID, OID>& wipCommits) const {
        if (hasWip) {
            // head should never be null if hasWip is true.
            // If the base is null, any commit would be a valid WIP.
            //
            // The first parent should always be the head of the base branch (if present),
            // even if the WIP commit is a merge.
            return !head.isNull && (base.isNull || repo.lookup_commit(wipCommits[head]).parent(0).id() == base);
        } else {
            return true;
        }
    }

    /**
     * Increment the version number of a branch name to the next unused one for that branch.
     *
     * @param name Name of the branch.
     * @param branchTargets List of targets for each branch.
     * @return The next valid branch name derivitive.
     */
    string next_conflict_branch_name(const string& name, const map<string, RefTargets>& branchTargets) {
        BranchDescriptor nextDesc(name);
        // Find the current greatest version number in use with this base name.
        for(const auto& entry : branchTargets) {
            BranchDescriptor d(entry.first);
            if (d.baseName == nextDesc.baseName) {
                nextDesc.version = max(nextDesc.version, d.version);
            }
        }
        // Increment to the next unused number.
        nextDesc.version++;
        return nextDesc.full_name();
    }

    OID wip_commit_hash(const Repository& repo, const OID& commit_oid) {
        Commit commit = repo.lookup_commit(commit_oid);

        git_oid message_hash;
        int err = git_odb_hash(&message_hash, commit.message().c_str(), commit.message().size(), GIT_OBJECT_BLOB);
        check_error(err);

        unsigned int size = (commit.parentcount() + 2) * GIT_OID_RAWSZ;
        unique_ptr<unsigned char> data(new unsigned char[size]);

        memcpy(data.get(), commit.tree().id().oid.id, GIT_OID_RAWSZ);
        memcpy(data.get() + GIT_OID_RAWSZ, message_hash.id, GIT_OID_RAWSZ);

        for (int i = 0; i < commit.parentcount(); i++) {
            memcpy(data.get() + (i + 2) * GIT_OID_RAWSZ, commit.parent(i).id().oid.id, GIT_OID_RAWSZ);
        }

        git_oid commit_hash;
        err = git_odb_hash(&commit_hash, data.get(), size, GIT_OBJECT_BLOB);
        check_error(err);

        return OID(commit_hash);
    }

    /**
     * Replace local and remote WIP branch targets with their corresponding WIP commit hashes, generated by
     * wip_commit_hash(). Synced targets are not affected, as they are already stored as WIP hashes.
     *
     * @param repo The repository.
     * @param branchTargets The targets to replace.
     * @param wipCommits Outputs a map from WIP hashes to a commit that has that hash,
     *        so that the underlying commit data can be retrieved.
     */
    void hash_wip_commits(const Repository& repo, map<string, RefTargets>& branchTargets, map<OID, OID>& wipCommits) {
        for(auto& entry : branchTargets) {
            const string branchName = entry.first;
            RefTargets targets = entry.second;

            if (targets.local.hasWip) {
                const OID wipHash = wip_commit_hash(repo, targets.local.head);
                wipCommits[wipHash] = targets.local.head;
                entry.second.local.head = wipHash;
            }
            if (targets.remote.hasWip) {
                const OID wipHash = wip_commit_hash(repo, targets.remote.head);
                wipCommits[wipHash] = targets.remote.head;
                entry.second.remote.head = wipHash;
            }
        }
    }

    /**
     * Create or overwrite a branch entry in the sync cache, which is stored at .git/synced/ for non-bare repos.
     * Like regular Git references, branch names with forward slashes in them are converted into nested
     * subdirectories.
     *
     * @param repo The repository.
     * @param name The name of the branch.
     * @param value The OID to store in the branch's cache file.
     */
    void create_cache_entry(const Repository& repo, const string& name, const OID& value) {
        // Create the sync cache root directory if it doesn't already exist.
        _set_errno(0);
        int err = _mkdir((repo.path() + "synced").c_str());
        if (err != 0 && errno != EEXIST) {
            throw MetroException("Failed to initialize sync cache");
        }

        // If the branch name contains slashes, create all the parent directories first.
        size_t pos = name.find('/');
        while (pos != string::npos) {
            _set_errno(0);
            err = _mkdir((repo.path() + "synced/" + name.substr(0, pos)).c_str());
            if (err != 0 && errno != EEXIST) {
                throw MetroException("Failed to create sync cache entry for " + name);
            }
            pos = name.find('/', pos+1);
        }

        write_all(value.str(), repo.path() + "synced/" + name);
    }

    /**
     * Delete a branch entry from the sync cache, if it exists.
     * Subdirectories within the sync cache directory that become empty after the deletion are also deleted.
     *
     * @param repo The repository.
     * @param name The branch to remove from the sync cache.
     */
    void delete_cache_entry(const Repository& repo, const string& name) {
        error_code ec;
        std_filesystem::remove((repo.path() + "synced/" + name).c_str(), ec);

        // Delete empty parent directories.
        size_t pos = name.find_last_of('/');
        while (pos != string::npos) {
            string directory = repo.path() + "synced/" + name.substr(0, pos);
            // Don't delete parent directories if they are not empty or do not exist.
            if (!std_filesystem::is_empty(directory, ec)) {
                break;
            }
            std_filesystem::remove_all(directory.c_str());

            pos = name.find_last_of('/', pos-1);
        }
    }

    /**
     * Updates the sync cache entries for the specified branches.
     *
     * Non-WIP branches have their commit OIDs stored.
     * WIP branches have their WIP commit hashes stored, as generated by wip_commit_hash().
     * Non-existent branches have their entries deleted.
     *
     * @param repo The repository.
     * @param branches Branch names for which to update the sync cache.
     */
    void update_sync_cache(const Repository& repo, const vector<string>& branches) {
        for (const auto& name : branches) {
            if (branch_exists(repo, name)) {
                OID oid = repo.lookup_branch(name, GIT_BRANCH_LOCAL).target();
                if (is_wip(name)) {
                    oid = wip_commit_hash(repo, oid);
                }
                create_cache_entry(repo, name, oid);
            } else {
                delete_cache_entry(repo, name);
            }
        }
    }

    /**
     * Read the entries in the sync cache (located at .git/synced/ in non-bare repos) into a map.
     * If the sync cache directory does not exist, the output map is unchanged.
     * Nested subdirectories within the sync cache directory are treated as branch names with forward slashes in them,
     * as with regular Git references.
     *
     * @param repo The repository.
     * @param out The map to write the entries to.
     */
    void read_sync_cache(const Repository& repo, map<string, OID>& out) {
        string cacheRoot = repo.path() + "synced";

        // Check that the sync cache directory exists before trying to read it.
        struct stat info{};
        _set_errno(0);
        int err = stat(cacheRoot.c_str(), &info);
        if (err != 0 && errno != ENOENT) {
            throw MetroException("Error accessing " + cacheRoot);
        }

        if (info.st_mode & S_IFDIR) {
            std_filesystem::recursive_directory_iterator end;
            std_filesystem::recursive_directory_iterator iter(cacheRoot);

            while (iter != end) {
                string path = iter->path().string();

                // Only try to read regular files; skip over directories returned by the iterator.
                if (std_filesystem::is_regular_file(iter->path())) {
                    // Remove the path prefix and following slash.
                    string name = path.substr(cacheRoot.size() + 1);
                    std::replace(name.begin(), name.end(), '\\', '/');
                    out[name] = OID(read_all(path));
                }

                error_code ec;
                iter.increment(ec);
                if (ec) {
                    throw MetroException("Error reading sync cache: " + ec.message());
                }
            }
        }
    }

    /**
     * Check if the given reference name starts with the specified prefix, and if so remove the prefix from the name
     * and ensure that there is an entry for that name in branchTargets by creating a blank one if needed.
     *
     * @param branchTargets List of targets for each branch.
     * @param name The name reference to remove the prefix from.
     * @param prefix Prefix to remove from name.
     * @return True if the prefix existed and was removed.
     */
    bool prepare_branch_targets(map<string, RefTargets>& branchTargets, string& name, const string& prefix) {
        if (has_prefix(name, prefix)) {
            name = name.substr(prefix.size(), name.size() - prefix.size());
            // Create a blank entry if none is present.
            if (branchTargets.find(name) == branchTargets.end()) {
                branchTargets[name] = RefTargets();
            }
            return true;
        }
        return false;
    }

    /**
     * Find the local, remote and sync-cached target OIDs of each local, remote and cached branch.
     * The local and remote targets will always be the actual commit OIDs, while the synced targets for WIP branches
     * will be WIP commit hashes as read from the sync cache.
     *
     * @param repo The repository.
     * @param out Output for the local, remote and synced targets for each branch.
     */
    void get_branch_targets(const Repository& repo, map<string, RefTargets> *out) {
        // Read targets from the sync cache.
        map<string, OID> synced;
        read_sync_cache(repo, synced);
        for (const auto& entry : synced) {
            string name = entry.first;
            bool isWip = is_wip(name);
            name = un_wip(name);

            // Create an empty RefTargets if none is present.
            if (out->find(name) == out->end()) {
                (*out)[name] = RefTargets();
            }
            (*out)[name].synced.add_target(entry.second, isWip);
        }

        // Read local and remote targets from repo.
        repo.foreach_reference([](const Branch& ref, const void *payload) {
            // Only try to sync direct references.
            if (ref.type() == GIT_REFERENCE_DIRECT) {
                auto branchTargets = (map<string, RefTargets> *) payload;

                // Base and WIP branches will be paired together in a DualTarget.
                string name = ref.reference_name();
                bool isWip = is_wip(name);
                name = un_wip(name);

                // Create an empty RefTargets if none is present and get the corresponding DualTarget from it.
                DualTarget *dualTarget = nullptr;
                if (prepare_branch_targets(*branchTargets, name, "refs/heads/")) {
                    dualTarget = &(*branchTargets)[name].local;
                } else if (prepare_branch_targets(*branchTargets, name, "refs/remotes/origin/")) {
                    dualTarget = &(*branchTargets)[name].remote;
                }

                if (dualTarget != nullptr) {
                    dualTarget->add_target(ref.target(), isWip);
                }
            }
            return 0;
        }, out);
    }

    /**
     * Refspec to push the specified branch, deleting the remote branch if requested.
     *
     * @param branchName Name of the branch to make push.
     * @param deleting Whether the remote branch should be deleted.
     * @return The final created refspec.
     */
    string make_push_refspec(const string& branchName, bool deleting) {
        if (deleting) {
            return ":refs/heads/" + branchName;
        } else {
            return "+refs/heads/" + branchName + ":refs/heads/" + branchName;
        }
    }

    /**
     * Add refspecs for pushing the specified base branch and its WIP branch.
     * Only pushes each branch if the local and remote targets differ.
     *
     * @param branchName Branch to queue up for push.
     * @param targets Targets to compare to branch.
     * @param refspecs Refspecs reference to add created refspec to
     */
    void queue_push(const string& branchName, const RefTargets& targets, vector<string>& refspecs) {
        if (targets.local.base != targets.remote.base) {
            refspecs.push_back(make_push_refspec(branchName, targets.local.base.isNull));
        }

        // If neither side has a WIP branch, don't try to push it.
        // If exactly one does, then push; in this case the heads are guaranteed to differ assuming valid WIP branch.
        // If both have WIP branches only push if the heads differ.
        if ((targets.local.hasWip || targets.remote.hasWip) && targets.local.head != targets.remote.head) {
            refspecs.push_back(make_push_refspec(to_wip(branchName), !targets.local.hasWip));
        }
    }
    
    /**
     * Move the specified branch to a new target.
     * Effectively performs a force pull if the new target is a commit fetched from a remote.
     * Can create and delete branches as needed.
     *
     * @param repo The repo to change the branch target within.
     * @param branchName The branch to move to a new target.
     * @param newTarget The new target the branch is moved to.
     */
    void change_branch_target(const Repository& repo, const string& branchName, const OID& newTarget) {
        if (newTarget.isNull) {
            // If this was a WIP branch it might already have been deleted when the base branch was deleted.
            if ((branch_exists(repo, branchName))) {
                delete_branch(repo, branchName);
            }
        } else {
            repo.create_reference("refs/heads/" + branchName, newTarget, true);
            // Update the working dir if this is the current branch.
            if (is_on_branch(repo, branchName)) {
                checkout(repo, branchName);
            }
        }
    }

    /**
     * Pull a fetched branch and its WIP counterpart by setting their targets to the fetched commits.
     *
     * @param repo Repo to fetch branch from.
     * @param branchName Branch to pull.
     * @param targets Targets to use use during pull.
     * @param wipCommits A mapping from WIP hashes (generated by wip_commit_hash())
     *        to the OID of a WIP commit with that hash.
     */
    void pull(const Repository& repo, const string& branchName, const RefTargets& targets, map<OID, OID>& wipCommits) {
        if (targets.local.base != targets.remote.base) {
            change_branch_target(repo, branchName, targets.remote.base);
        }

        // If neither side has a WIP branch, don't try to pull it.
        // If exactly one does, then pull; in this case the heads are guaranteed to differ assuming valid WIP branch.
        // If both have WIP branches only pull if the heads differ.
        if ((targets.local.hasWip || targets.remote.hasWip) && targets.local.head != targets.remote.head) {
            change_branch_target(repo, to_wip(branchName), targets.remote.hasWip? wipCommits[targets.remote.head] : OID());
        }
    }

    /**
     * Move the local commits of a conflicting branch to a new branch, then pull the remote commits into the old branch.
     * The new branch with the local changes is scheduled to be pushed to remote.
     *
     * @param repo The repo to create branches within.
     * @param remote The remote to fetch from.
     * @param name The name of the branch the conflict occurs within.
     * @param localTarget The local branch OID.
     * @param remoteTarget The remote branch OID.
     * @param direction The direction syncing is occurring.
     * @param branchTargets The local, remote and synced targets for each branch.
     * @param conflictBranchNames The list of existing conflict branch names.
     * @param pushRefspecs List of refspecs to push.
     * @param syncedBranches List of branches which have been synced.
     * @param wipCommits A mapping from WIP hashes (generated by wip_commit_hash())
     *        to the OID of a WIP commit with that hash.
     */
    void create_conflict_branches(const Repository& repo, const Remote& remote, const string& name,
            const RefTargets& targets, const SyncDirection& direction, const map<string, RefTargets>& branchTargets,
            vector<string>& pushRefspecs, vector<string>& syncedBranches, map<OID, OID>& wipCommits) {
        assert(direction != UP);  // Should never try to sync conflicting branches with --push

        // Generate the new branch name.
        string newName = next_conflict_branch_name(name, branchTargets);

        repo.create_reference("refs/heads/" + newName, targets.local.base, false);
        if (targets.local.hasWip) {
            repo.create_reference("refs/heads/" + to_wip(newName), wipCommits[targets.local.head], false);
        }

        cout << "Branch " << name << " had remote changes that conflicted with yours; your commits have been moved to " << newName << "." << endl;
        // If this is the current branch, move the head to the new branch
        // so the user stays on their version of the branch.
        // We don't need to checkout as the contents will not have changed.
        if (is_on_branch(repo, name)) {
            move_head(repo, newName);
            cout << "You've been moved to " << newName << "." << endl;
        }

        // Pull the remote branch under the original branch name.
        pull(repo, name, targets, wipCommits);
        syncedBranches.push_back(name);
        syncedBranches.push_back(to_wip(name));

        if (direction != DOWN) {
            pushRefspecs.push_back(make_push_refspec(newName, false));
            if (targets.local.hasWip) {
                pushRefspecs.push_back(make_push_refspec(to_wip(newName), false));
            }

            syncedBranches.push_back(newName);
            syncedBranches.push_back(to_wip(newName));
        }
    }

    /**
     * Callback for push transfer.
     */
    int push_transfer_progress(unsigned int current, unsigned int total, size_t bytes, void *payload) {
        if (total > 0) {
            unsigned int progress = (100 * current) / total;
            print_progress(progress, bytes);
        }

        return GIT_OK;
    }

    /**
     * Callback for fetch transfer.
     */
    int transfer_progress(const git_transfer_progress* stats, void* payload) {
        unsigned int progress = (100 * (stats->received_objects + stats->indexed_objects)) / (2 * stats->total_objects);
        print_progress(progress, stats->received_bytes);

        if (exit_config.cancel) {
            exit_config.received = true;
            return GIT_ERROR;
        }

        return GIT_OK;
    }


    Repository clone(const string& url, const string& path) {
        CredentialStore credentials;
        try {
            Repository repo = clone(url, path, &credentials);
            return repo;
        } catch (GitException &e) {
            string error(e.what());
            string test;
            int pos;

            test = "The filename, directory name, or volume label syntax is incorrect.";
            pos = error.find(test) + string(test).length();
            if (pos >= 0 && pos <= error.length()) {
                throw MetroException("Unable to create directory '" + path + "' as that name is disallowed on this OS.");
            }
            throw e;
        }
    }

    Repository clone(const string& url, const string& path, CredentialStore *credentials) {
        const string repoPath = path + "/.git";
        if (Repository::exists(repoPath)) {
            throw RepositoryExistsException();
        }

        git_clone_options options = GIT_CLONE_OPTIONS_INIT;
        options.fetch_opts.callbacks.credentials = acquire_credentials;
        CredentialPayload payload{credentials, nullptr};
        options.fetch_opts.callbacks.payload = &payload;
        options.fetch_opts.callbacks.transfer_progress = transfer_progress;

        credentials->tried = false;
        exit_config.started = true;
        Repository repo = git::Repository::clone(url, repoPath, &options);
        // Pull all the other branches (which were fetched anyway).
        force_pull(repo);
        clear_progress_bar();
        return repo;
    }

    void sync(const Repository& repo, SyncDirection direction, bool force) {
        CredentialStore credentials;
        sync(repo, &credentials, direction, force);
    }

    void sync(const Repository& repo, CredentialStore *credentials, SyncDirection direction, bool force) {
        save_wip(repo);

        git_fetch_options fetchOpts = GIT_FETCH_OPTIONS_INIT;
        fetchOpts.prune = GIT_FETCH_PRUNE;
        fetchOpts.callbacks.credentials = acquire_credentials;
        CredentialPayload payload = {credentials, &repo};
        fetchOpts.callbacks.payload = &payload;
        fetchOpts.callbacks.transfer_progress = transfer_progress;

        Remote origin = repo.lookup_remote("origin");
        cout << "Syncing with " << git_remote_url(origin.ptr().get()) << "." << endl;
        credentials->tried = false;
        cout << "Fetching all branches from remote..." << endl;
        origin.fetch(StrArray(), fetchOpts);
        clear_progress_bar();

        map<string, RefTargets> branchTargets;
        get_branch_targets(repo, &branchTargets);

        // Replace WIP commit OIDs with the WIP hashes of those commits.
        // This ensures that when we compare WIP commits later irrelevant metadata
        // such as timestamps and authors are ignored.
        map<OID, OID> wipCommits;
        hash_wip_commits(repo, branchTargets, wipCommits);

        vector<string> pushRefspecs;
        // Branches that are known to have matching targets on remote and local after this sync operation.
        vector<string> syncedBranches;
        for(const auto& entry : branchTargets) {
            const string branchName = entry.first;
            const RefTargets targets = entry.second;

            if (targets.local.base == targets.remote.base) {
                syncedBranches.push_back(branchName);
            }

            if (targets.local.head == targets.remote.head) {
                if (targets.local.hasWip) {
                    syncedBranches.push_back(to_wip(branchName));
                }

                if (is_on_branch(repo, branchName)) {
                    cout << "Branch " << branchName << " is already synced." << endl;
                }
            } else if (targets.local.is_valid(repo, wipCommits) && targets.remote.is_valid(repo, wipCommits)) {
                SyncType syncType;

                if (targets.local.head == targets.synced.head) {
                    // Only remote has changed so pull.
                    syncType = PULL;
                } else if (targets.remote.head == targets.synced.head) {
                    // Only local has changed so push.
                    syncType = PUSH;
                } else {
                    // Both sides have changed since last sync.
                    syncType = CONFLICT;
                }

                // If the local and remote branches are found to be in conflict, check whether the head of one
                // is an ancestor of the other. In this case, rather than making a new conflict branch that contains
                // no commits since the divergence point, just retain all the commits on both sides.
                if (syncType == CONFLICT) {
                    // Find the most recent common ancestor. Will default to null if they have none in common,
                    // or one of the branches does not exist.
                    OID base;
                    if (!(targets.local.head.isNull || targets.remote.head.isNull)) {
                        try {
                            base = repo.merge_base(targets.local.head, targets.remote.head);
                        } catch (GitException& ex) {
                            // If there is no base at all, keep it as null.
                        }
                    }

                    if (targets.local.head == base) {
                        // Remote has more commits so pull.
                        cout << "Branch " << branchName << " has been modified both locally and remotely, "
                             << "but in different ways. The local branch has been updated." << endl;
                        syncType = PULL;
                    } else if (targets.remote.head == base) {
                        // Local has more commits so push.
                        cout << "Branch " << branchName << " has been modified both locally and remotely, "
                             << "but in different ways. The remote branch has been updated." << endl;
                        syncType = PUSH;
                    } else {
                        // Neither side is an ancestor of the other so a new conflict branch must be made.
                        syncType = CONFLICT;
                    }
                }

                switch (syncType) {
                    case PUSH:
                        if (direction == UP || direction == BOTH) {
                            cout << "Pushing " << branchName << "..." << endl;
                            queue_push(branchName, targets, pushRefspecs);

                            syncedBranches.push_back(branchName);
                            syncedBranches.push_back(to_wip(branchName));
                        }
                        break;
                    case PULL:
                        if (direction == DOWN || direction == BOTH) {
                            cout << "Pulling " << branchName << "..." << endl;
                            pull(repo, branchName, targets, wipCommits);

                            syncedBranches.push_back(branchName);
                            syncedBranches.push_back(to_wip(branchName));
                        }
                        break;
                    case CONFLICT:
                        if (direction != UP) {
                            create_conflict_branches(repo, origin, branchName, targets, direction,
                                                     branchTargets, pushRefspecs, syncedBranches, wipCommits);
                        } else {
                            cout << "Branch " << branchName << " conflicts with remote, not pushing." << endl;
                        }
                        break;
                }
            } else {
                // Don't attempt to sync a broken WIP branch,
                // as it is hard to tell what the user intended in such a situation.
                bool localOK = targets.local.is_valid(repo, wipCommits);
                string side = localOK ? "Remote" : "Local";
                cout << side << " wip branch for " << branchName << " is not a valid work in progress branch for "
                     << branchName << ", so neither branch can be synced. Delete " << to_wip(branchName)
                     << " to resolve the issue." << endl;
            }
        }

        if (!pushRefspecs.empty()) {
            // Pushes shouldn't be queued in the first place when using --pull.
            assert(direction == UP || direction == BOTH);

            git_push_options options = GIT_PUSH_OPTIONS_INIT;
            options.callbacks.credentials = acquire_credentials;
            options.callbacks.payload = &payload;
            options.callbacks.push_transfer_progress = push_transfer_progress;

            credentials->tried = false;
            origin.push(StrArray(pushRefspecs), options);
            clear_progress_bar();
        }

        update_sync_cache(repo, syncedBranches);
        restore_wip(repo);
    }

    void force_pull(const Repository& repo) {
        map<string, RefTargets> branchTargets;
        get_branch_targets(repo, &branchTargets);

        map<OID, OID> wipCommits;
        hash_wip_commits(repo, branchTargets, wipCommits);

        vector<string> syncedBranches;
        for(const auto& entry : branchTargets) {
            const string branchName = entry.first;
            const RefTargets targets = entry.second;
            pull(repo, branchName, targets, wipCommits);

            syncedBranches.push_back(branchName);
            syncedBranches.push_back(to_wip(branchName));
        }

        update_sync_cache(repo, syncedBranches);
    }
}