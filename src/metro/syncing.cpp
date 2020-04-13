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

    bool DualTarget::is_valid(const Repository& repo) const {
        if (hasWip) {
            // head should never be null if hasWip is true.
            // base should not be null if hasWip is true, as that would imply there is a WIP branch with no base branch.
            //
            // The first parent should always be the head of the base branch,
            // even if the WIP commit is a merge.
            return (!base.isNull && ! head.isNull) && repo.lookup_commit(head).parent(0).id() == base;
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

    /**
     * Returns true if both OIDs are non-null and point to commits with identical trees.
     * The message and other metadata may be different.
     *
     * @param repo Repo to test within.
     * @param oid1 First OID to test.
     * @param oid2 Second OID to test.
     * @return True if both ID's have identical final trees.
     */
    bool commit_contents_identical(const Repository& repo, const OID& oid1, const OID& oid2) {
        if (oid1.isNull || oid2.isNull) {
            return false;
        }

        Commit commit1 = repo.lookup_commit(oid1);
        Commit commit2 = repo.lookup_commit(oid2);

        Diff diff = Diff::tree_to_tree(repo, commit1.tree(), commit2.tree(), nullptr);
        // No differences.
        return diff.num_deltas() == 0;
    }

    /**
     * Updates the sync cache to mark the passed branches as updated.
     *
     * @param repo Repo to save cache in.
     * @param branches Branches which have been updated.
     */
    void update_sync_cache(const Repository& repo, const vector<string>& branches) {
        for (const auto& name : branches) {
            if (branch_exists(repo, name)) {
                Branch branch = repo.lookup_branch(name, GIT_BRANCH_LOCAL);
                repo.create_reference("refs/synced/" + name, branch.target(), true);
            } else {
                Branch ref = repo.lookup_reference("refs/synced/" + name);
                ref.delete_reference();
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
     *
     * @param repo Repo to find targets in.
     * @param out List of targets for each branch.
     */
    void get_branch_targets(const Repository& repo, const map<string, RefTargets> *out) {
        repo.foreach_reference([](const Branch& ref, const void *payload) {
            auto branchTargets = (map<string, RefTargets>*) payload;

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
            } else if (prepare_branch_targets(*branchTargets, name, "refs/synced/")) {
                dualTarget = &(*branchTargets)[name].synced;
            }

            if (dualTarget != nullptr) {
                dualTarget->add_target(ref.target(), isWip);
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
            if (branchName == current_branch_name(repo)) {
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
     */
    void pull(const Repository& repo, const string& branchName, const RefTargets& targets) {
        if (targets.local.base != targets.remote.base) {
            change_branch_target(repo, branchName, targets.remote.base);
        }

        // If neither side has a WIP branch, don't try to pull it.
        // If exactly one does, then pull; in this case the heads are guaranteed to differ assuming valid WIP branch.
        // If both have WIP branches only pull if the heads differ.
        if ((targets.local.hasWip || targets.remote.hasWip) && targets.local.head != targets.remote.head) {
            change_branch_target(repo, to_wip(branchName), targets.remote.hasWip? targets.remote.head : OID());
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
     * @param branchTargets The list of targets for each branch.
     * @param conflictBranchNames The list of existing conflict branch names.
     * @param pushRefspecs List of refspects to push.
     * @param syncedBranches List of branches which have been synced
     */
    void create_conflict_branches(const Repository& repo, const Remote& remote, const string& name,
            const RefTargets& targets, const SyncDirection& direction, const map<string, RefTargets>& branchTargets,
            vector<string>& pushRefspecs, vector<string>& syncedBranches) {
        assert(direction != UP);  // Should never try to sync conflicting branches with --push

        // Generate the new branch name.
        string newName = next_conflict_branch_name(name, branchTargets);

        repo.create_reference("refs/heads/" + newName, targets.local.base, false);
        if (targets.local.hasWip) {
            repo.create_reference("refs/heads/" + to_wip(newName), targets.local.head, false);
        }

        cout << "Branch " << name << " had remote changes that conflicted with yours; your commits have been moved to " << newName << "." << endl;
        // If this is the current branch, move the head to the new branch
        // so the user stays on their version of the branch.
        // We don't need to checkout as the contents will not have changed.
        if (name == current_branch_name(repo)) {
            move_head(repo, newName);
            cout << "You've been moved to " << newName << "." << endl;
        }

        // Pull the remote branch under the original branch name.
        pull(repo, name, targets);
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
        unsigned int progress = (100 * current) / total;
        print_progress(progress, bytes);

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
        Repository repo = clone(url, path, &credentials);
        return repo;
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

                if (branchName == current_branch_name(repo)) {
                    cout << "Branch " << branchName << " is already synced." << endl;
                }
            } else if (targets.local.is_valid(repo) && targets.remote.is_valid(repo)) {
                // If the local and remote branches both have WIP branches, and the contents of the WIP commits
                // are the same, the decision about which syncing action to perform should be delegated to the base
                // branches instead of the heads as would usually be done. This ensures that if the WIP commits are
                // technically different commits but have the same contents, an unnecessary conflict resolution between
                // them is avoided.

                OID uniqueLocal = targets.local.head;
                OID uniqueRemote = targets.remote.head;
                OID uniqueSynced = targets.synced.head;

                if (targets.local.hasWip && targets.remote.hasWip
                    && commit_contents_identical(repo, targets.local.head, targets.remote.head)) {

                    uniqueLocal = targets.local.base;
                    uniqueRemote = targets.remote.base;
                    uniqueSynced = targets.synced.base;
                }

                SyncType syncType;

                // If the unique heads are the same, but the true heads differ, this means the only difference
                // in the branches is the metadata of the WIP commit. Therefore just pull the remote version
                // to keep them synced.
                if (uniqueLocal == uniqueRemote) {
                    syncType = PULL;
                } else if (!targets.synced.is_valid(repo)) {
                    // If the local and remote dual branches are valid, but synced is not, then both have changed
                    // since the last sync. This means there is a conflict if they are not already synced.
                    // uniqueSynced cannot be trusted for the below checks if it is not valid.
                    syncType = CONFLICT;
                } else if (uniqueLocal == uniqueSynced) {
                    // Only remote has changed so pull.
                    syncType = PULL;
                } else if (uniqueRemote == uniqueSynced) {
                    // Only local has changed so push.
                    syncType = PUSH;
                } else {
                    OID base;
                    if (!(uniqueLocal.isNull || uniqueRemote.isNull)) {
                        base = repo.merge_base(uniqueLocal, uniqueRemote);
                    }

                    if (uniqueLocal == base) {
                        cout << "Branch " << branchName << " has been modified both locally and remotely, "
                             << "but in different ways. The local branch has been updated." << endl;
                        syncType = PULL;
                    } else if (uniqueRemote == base) {
                        cout << "Branch " << branchName << " has been modified both locally and remotely, "
                             << "but in different ways. The remote branch has been updated." << endl;
                        syncType = PUSH;
                    } else {
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
                            pull(repo, branchName, targets);

                            syncedBranches.push_back(branchName);
                            syncedBranches.push_back(to_wip(branchName));
                        }
                        break;
                    case CONFLICT:
                        if (direction != UP) {
                            create_conflict_branches(repo, origin, branchName, targets, direction,
                                                     branchTargets, pushRefspecs, syncedBranches);
                        } else {
                            cout << "Branch " << branchName << " conflicts with remote, not pushing." << endl;
                        }
                        break;
                }
            } else {
                // Don't attempt to sync a broken WIP branch,
                // as it is hard to tell what the user intended in such a situation.
                bool localOK = targets.local.is_valid(repo);
                string side = localOK ? "Remote" : "Local";
                if ((localOK? targets.remote : targets.local).base.isNull) {
                    cout << side << " wip branch for " << branchName << " has no corresponding base branch "
                         << branchName << ". Therefore it cannot be synced." << endl;
                } else {
                    cout << side << " wip branch for " << branchName << " is not a valid work in progress branch for "
                         << branchName << ", so neither branch can be synced. Delete " << to_wip(branchName)
                         << " to resolve the issue." << endl;
                }
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

        vector<string> syncedBranches;
        for(const auto& entry : branchTargets) {
            const string branchName = entry.first;
            const RefTargets targets = entry.second;
            pull(repo, branchName, targets);

            syncedBranches.push_back(branchName);
            syncedBranches.push_back(to_wip(branchName));
        }

        update_sync_cache(repo, syncedBranches);
    }
}