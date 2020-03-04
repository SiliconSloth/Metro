#include "pch.h"

namespace metro {
    struct RefTargets {
        OID local;
        OID remote;
        OID synced;
    };

    enum SyncType {PUSH, PULL, CONFLICT};

    // Increment the version number of a branch name to the next unused one for that branch.
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

    // Returns true if both OIDs are non-null and point to commits with identical trees.
    // The message and other metadata may be different.
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

    void clear_sync_cache(const Repository& repo) {
        repo.foreach_reference([](const Branch& ref, const void *payload) {
            if (has_prefix(ref.reference_name(), "refs/synced/")) {
                ref.delete_reference();
            }
            return 0;
        }, nullptr);
    }

    void update_sync_cache(const Repository& repo) {
        clear_sync_cache(repo);

        BranchIterator iter = repo.new_branch_iterator(GIT_BRANCH_LOCAL);
        for (Branch branch; iter.next(&branch);) {
            repo.create_reference("refs/synced/" + branch.name(), branch.target(), false);
        }
    }

    // Check if the given reference name starts with the specified prefix, and if so remove the prefix from the name
    // and ensure that there is an entry for that name in branchTargets by creating a blank one if needed.
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

    // Find the local, remote and sync-cached target OIDs of each local, remote and cached branch.
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

    // Refspec to push the specified branch, deleting the remote branch if requested.
    string make_push_refspec(const string& branchName, bool deleting) {
        if (deleting) {
            return ":refs/heads/" + branchName;
        } else {
            return "+refs/heads/" + branchName + ":refs/heads/" + branchName;
        }
    }

    // Move the specified branch to a new target.
    // Effectively performs a force pull if the new target is a commit fetched from a remote.
    // Can create and delete branches as needed.
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

    // Move the local commits of a conflicting branch to a new branch, then pull the remote commits into the old branch.
    // The new branch with the local changes is scheduled to be pushed to remote.
    void create_conflict_branches(const Repository& repo, const Remote& remote, const string& name,
            const OID& localTarget, const OID& remoteTarget, const map<string, RefTargets>& branchTargets,
            map<string, string>& conflictBranchNames, vector<string>& pushRefspecs) {

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
        pushRefspecs.push_back(make_push_refspec(newName, false));
        cout << "Branch " << name << " had remote changes that conflicted with yours, your commits have been moved to " << newName << ".\n";
    }

    int transfer_progress(const git_transfer_progress* stats, void* payload) {
        int progress = (100 * (stats->received_objects + stats->indexed_objects)) / (2 * stats->total_objects);

#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        int width = csbi.srWindow.Right - csbi.srWindow.Left - 16;
#elif __unix__
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int width = w.ws_col - 17;
#endif

        string bar;
        int i;
        if (width > 0) {
            for (i = 0; i < (progress * width) / 100; i++) {
                bar.append("=");
            }
            for (; i < width; i++) {
                bar.append("-");
            }
            cout << "\r" << "Progress: [" << bar << "] " << progress << "%" << flush;
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
        Repository repo = git::Repository::clone(url, repoPath, &options);
        // Pull all the other branches (which were fetched anyway).
        force_pull(repo);
        return repo;
    }

    void sync(const Repository& repo) {
        CredentialStore credentials;
        sync(repo, &credentials);
    }

    void sync(const Repository& repo, CredentialStore *credentials) {
        save_wip(repo);

        git_fetch_options fetchOpts = GIT_FETCH_OPTIONS_INIT;
        fetchOpts.prune = GIT_FETCH_PRUNE;
        fetchOpts.callbacks.credentials = acquire_credentials;
        CredentialPayload payload = {credentials, &repo};
        fetchOpts.callbacks.payload = &payload;

        Remote origin = repo.lookup_remote("origin");
        credentials->tried = false;
        cout << "Fetching changes..." << endl;
        origin.fetch(StrArray(), fetchOpts);

        map<string, RefTargets> branchTargets;
        get_branch_targets(repo, &branchTargets);

        map<string, string> conflictBranchNames;
        vector<string> pushRefspecs;
        for(const auto& entry : branchTargets) {
            const string branchName = entry.first;
            const RefTargets targets = entry.second;

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
                        cout << "Branch " << branchName << " has been modified both locally and remotely, "
                             << "but in different ways. All remote commits will be retained locally." << endl;
                        syncType = PULL;
                    } else if (targets.remote == base) {
                        cout << "Branch " << branchName << " has been modified both locally and remotely, "
                             << "but in different ways. All local commits will be retained remotely." << endl;
                        syncType = PUSH;
                    } else {
                        syncType = CONFLICT;
                    }
                }

                switch (syncType) {
                    case PUSH:
                        cout << "Pushing " << branchName << endl;
                        pushRefspecs.push_back(make_push_refspec(branchName, targets.local.isNull));
                        break;
                    case PULL:
                        cout << "Pulling " << branchName << endl;
                        change_branch_target(repo, branchName, targets.remote);
                        break;
                    case CONFLICT:
                        create_conflict_branches(repo, origin, branchName, targets.local, targets.remote,
                                branchTargets, conflictBranchNames, pushRefspecs);
                        break;
                }
            } else {
                cout << "Branch " << branchName << " is already synced" << endl;
            }
        }

        // Make sure that if a new WIP branch was created, the corresponding base branch is also created.
        for(const auto& entry : conflictBranchNames) {
            const string oldName = entry.first;
            const string newName = entry.second;

            if (branch_exists(repo, oldName) &&! branch_exists(repo, newName)) {
                OID target = repo.lookup_branch(oldName, GIT_BRANCH_LOCAL).target();
                // Create branch pointing to same commit as old branch.
                create_conflict_branches(repo, origin, oldName, target, target,
                        branchTargets, conflictBranchNames, pushRefspecs);
            }
        }

        if (!pushRefspecs.empty()) {
            PushOptions options = GIT_PUSH_OPTIONS_INIT;
            options.callbacks.credentials = acquire_credentials;
            options.callbacks.payload = &payload;

            credentials->tried = false;
            cout << "Pushing changes..." << endl;
            origin.push(StrArray(pushRefspecs), options);
        }

        update_sync_cache(repo);
        restore_wip(repo);
    }

    // Pulls all the repo branches assuming the remote is correct
    void force_pull(const Repository& repo) {
        map<string, RefTargets> branchTargets;
        get_branch_targets(repo, &branchTargets);
        for(const auto& entry : branchTargets) {
            const string branchName = entry.first;
            const RefTargets targets = entry.second;
            change_branch_target(repo, branchName, targets.remote);
        }
        update_sync_cache(repo);
    }
}