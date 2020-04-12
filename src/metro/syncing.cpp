namespace metro {

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
                branchTargets[name] = {OID(), OID(), OID()};
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
            string name = ref.reference_name();
            auto branchTargets = (map<string, RefTargets>*) payload;

            if (prepare_branch_targets(*branchTargets, name, "refs/heads/")) {
                (*branchTargets)[name].local = ref.target();
            } else if (prepare_branch_targets(*branchTargets, name, "refs/remotes/origin/")) {
                (*branchTargets)[name].remote = ref.target();
            } else if (prepare_branch_targets(*branchTargets, name, "refs/synced/")) {
                (*branchTargets)[name].synced = ref.target();
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
            const OID& localTarget, const OID& remoteTarget, const SyncDirection& direction,
            const map<string, RefTargets>& branchTargets, map<string, string>& conflictBranchNames,
            vector<string>& pushRefspecs, vector<string>& syncedBranches) {
        assert(direction != UP);  // Should never try to sync conflicting branches with --push

        // If a new name has already been generated for this branch (or the base branch if this is a WIP branch),
        // use the existing new name. Otherwise generate a new one.
        const string nameUnwipped = un_wip(name);
        string newName;
        if (conflictBranchNames.find(nameUnwipped) != conflictBranchNames.end()) {
            newName = conflictBranchNames[nameUnwipped];
            if (is_wip(name)) {
                newName = to_wip(newName);
            }
        } else {
            newName = next_conflict_branch_name(name, branchTargets);
            conflictBranchNames[nameUnwipped] = un_wip(newName);
        }

        repo.create_reference("refs/heads/" + newName, localTarget, false);
        // If this is the current branch, move the head to the new branch
        // so the user stays on their version of the branch.
        // We don't need to checkout as the contents will not have changed.
        if (name == current_branch_name(repo)) {
            move_head(repo, newName);
        }

        // Point the old branch to the fetched remote commits.
        repo.create_reference("refs/heads/" + name, remoteTarget, true);
        syncedBranches.push_back(name);
        if (direction != DOWN) {
            pushRefspecs.push_back(make_push_refspec(newName, false));
            syncedBranches.push_back(newName);
        }
        cout << "Branch " << name << " had remote changes that conflicted with yours, your commits have been moved to " << newName << ".\n";
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
        cout << "Fetching all branches from origin..." << endl;
        origin.fetch(StrArray(), fetchOpts);
        clear_progress_bar();

        map<string, RefTargets> branchTargets;
        get_branch_targets(repo, &branchTargets);

        map<string, string> conflictBranchNames;
        vector<string> pushRefspecs;
        vector<string> syncedBranches;
        for(const auto& entry : branchTargets) {
            const string branchName = entry.first;
            const RefTargets targets = entry.second;

            // Make nicer branch name to print for WIP
            string printBranchName = branchName;
            bool isWIP = is_wip(branchName);
            if (isWIP) {
                printBranchName = un_wip(branchName) + " wip branch";
            }

            if (targets.local != targets.remote) {
                SyncType syncType;

                // If WIP commits have identical contents (but possibly different metadata)
                // just keep the remote one.
                if (is_wip(branchName) && commit_contents_identical(repo, targets.local, targets.remote)) {
                    syncType = PULL;
                } else if (targets.local == targets.synced) {
                    syncType = PULL;
                } else if (targets.remote == targets.synced) {
                    syncType = PUSH;
                } else {
                    OID base;
                    if (!targets.local.isNull && !targets.remote.isNull) {
                        base = repo.merge_base(targets.local, targets.remote);
                    }

                    if (targets.local == base) {
                        cout << "Branch " << printBranchName << " has been modified both locally and remotely, "
                             << "but in different ways. The local branch has been updated." << endl;
                        syncType = PULL;
                    } else if (targets.remote == base) {
                        cout << "Branch " << printBranchName << " has been modified both locally and remotely, "
                             << "but in different ways. The remote branch has been updated." << endl;
                        syncType = PUSH;
                    } else {
                        syncType = CONFLICT;
                    }
                }

                switch (syncType) {
                    case PUSH:
                        if (direction == UP || direction == BOTH) {
                            cout << "Pushing " << printBranchName << " to origin/" << printBranchName << "..." << endl;
                            pushRefspecs.push_back(make_push_refspec(branchName, targets.local.isNull));
                            syncedBranches.push_back(branchName);
                        }
                        break;
                    case PULL:
                        if (direction == DOWN || direction == BOTH) {
                            cout << "Pulling from origin/" << printBranchName << " to " << printBranchName << "..." << endl;
                            change_branch_target(repo, branchName, targets.remote);
                            syncedBranches.push_back(branchName);
                        }
                        break;
                    case CONFLICT:
                        if (direction != UP) {
                            create_conflict_branches(repo, origin, branchName, targets.local, targets.remote, direction,
                                                     branchTargets, conflictBranchNames, pushRefspecs, syncedBranches);
                        } else {
                            cout << "Branch " << branchName << " conflicts with remote, not pushing." << endl;
                        }
                        break;
                }
            } else {
                if (branchName == current_branch_name(repo)) {
                    if (!isWIP) {
                        cout << "Branch " << branchName << " is already synced." << endl;
                    } else {
                        cout << "WIP branch " << branchName << " is already synced." << endl;
                    }
                }
            }
        }

        // Make sure that if a new WIP branch was created, the corresponding base branch is also created.
        for(const auto& entry : conflictBranchNames) {
            const string oldName = entry.first;
            const string newName = entry.second;

            if (branch_exists(repo, oldName) &&! branch_exists(repo, newName)) {
                OID target = repo.lookup_branch(oldName, GIT_BRANCH_LOCAL).target();
                // Create branch pointing to same commit as old branch.
                create_conflict_branches(repo, origin, oldName, target, target, direction,
                        branchTargets, conflictBranchNames, pushRefspecs, syncedBranches);
            }
        }

        if (!pushRefspecs.empty() && (direction == UP || direction == BOTH)) {
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
            change_branch_target(repo, branchName, targets.remote);
            syncedBranches.push_back(branchName);
        }

        update_sync_cache(repo, syncedBranches);
    }
}