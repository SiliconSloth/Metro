#include "pch.h"

namespace metro {
    struct RefTargets {
        OID local;
        OID remote;
        OID synced;
    };

    enum SyncType {PUSH, PULL, CONFLICT};

    // Extract the name of a remote repo from its URL.
    // Roughly speaking this is the last component of the path,
    // excluding any final .git component or .git or .bundle file extension.
    string name_from_url(const string& url) {
        // Treat all types of slashes the same.
        string name = url;
        replace(name.begin(), name.end(), '\\', '/');

        // Remove the protocol if present.
        size_t protocolStop = url.find("://");
        if (protocolStop != string::npos) {
            name = name.substr(protocolStop+3, name.size() - (protocolStop+3));
        }

        // If a username is provided in the form user@server/something/repo, remove it.
        const regex authPattern("^[^\\/]*@");
        smatch matches;
        regex_search(name, matches, authPattern);
        // Since the regex starts with ^ there should be at most one match.
        for (auto match : matches) {
            cout << "name" << endl;
            name = name.substr(match.length(), name.size() - match.length());
            break;
        }

        // Get the final component of the URL path, ignoring any empty/whitespace only components.
        // If the last non-empty component is ".git" it will be ignored, but any prior ".git"s
        // will be kept.
        size_t lastSlash;
        string lastComponent;
        bool skippedGit = false;
        while (true) {
            lastSlash = name.find_last_of('/');
            if (lastSlash != string::npos) {
                lastComponent = name.substr(lastSlash+1, name.size() - (lastSlash+1));
                name = name.substr(0, lastSlash);
            } else {
                // Stop searching once the first component of the URL is reached,
                // even if it is invalid (that will be checked later).
                break;
            }

            // If this component is suitable, stop searching.
            if (!whitespace_only(lastComponent) && (lastComponent != ".git" || skippedGit)) {
                name = lastComponent;
                break;
            }
            // Only skip the first ".git".
            if (lastComponent == ".git") {
                skippedGit = true;
            }
        }

        // If the repo name ends with ".git" or ".bundle", but this is not the entire name,
        // remove the extension.
        if (has_suffix(name, ".git") && name != ".git") {
            name = name.substr(0, name.size() - 4);
        } else if (has_suffix(name, ".bundle") && name != ".bundle") {
            name = name.substr(0, name.size() - 7);
        }

        if (whitespace_only(name)) {
            throw UnsupportedOperationException("Couldn't find repository name in URL.");
        }

        return name;
    }

    // Increment the version number of a branch name to the next unused one for that branch.
    string next_conflict_branch_name(const Repository& repo, const string& name) {
        BranchDescriptor nextDesc(name);
        BranchIterator iter = repo.new_branch_iterator(GIT_BRANCH_LOCAL);
        // Find the current greatest version number in use with this base name.
        for (Branch branch; iter.next(&branch);) {
            BranchDescriptor d(branch.name());
            if (d.baseName == nextDesc.baseName) {
                nextDesc.version = max(nextDesc.version, d.version);
            }
        }
        // Increment to the next unused number.
        nextDesc.version++;
        return nextDesc.full_name();
    }

    int acquire_credentials(git_cred **cred, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload) {
        string username;
        string password;
        switch (allowed_types) {
            case GIT_CREDTYPE_DEFAULT:
                return git_cred_default_new(cred);
            case GIT_CREDTYPE_USERPASS_PLAINTEXT:
                cout << "Username for " << url << ": ";
                getline(cin, username);
                cout << "Password for " << username << ": ";
                password = read_password();

                return git_cred_userpass_plaintext_new(cred, username.c_str(), password.c_str());
            default:
                cout << "Metro currently doesn't support SSH. Please use HTTPS.";
                return GIT_ERROR;
        }
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

    // Force push the specified branch to the remote, deleting the remote branch if requested.
    void push(const Remote& remote, const string& branchName, bool deleting) {
        PushOptions opts = GIT_PUSH_OPTIONS_INIT;
        opts.callbacks.credentials = acquire_credentials;
        string refspec = deleting? ":refs/heads/" + branchName : "+refs/heads/" + branchName + ":refs/heads/" + branchName;
        remote.push(StrArray({refspec}), opts);
        cout << "Pushed to remote " << branchName << "." << endl;
    }

    // Move the specified branch to a new target.
    // Effectively performs a force pull if the new target is a commit fetched from a remote.
    // Can create and delete branches as needed.
    void change_branch_target(const Repository& repo, const string& branchName, const OID& newTarget) {
        if (newTarget.isNull) {
            delete_branch(repo, branchName);
        } else {
            repo.create_reference("refs/heads/" + branchName, newTarget, true);
            // Update the working dir if this is the current branch.
            if (branchName == current_branch_name(repo)) {
                checkout(repo, branchName);
            }
        }
    }

    // Move the local commits of a conflicting branch to a new branch, then pull the remote commits into the old branch.
    // The new branch with the local changes is pushed to remote.
    void create_conflict_branches(const Repository& repo, const Remote& remote, const string& name, const OID& localTarget, const OID& remoteTarget) {
        const string newName = next_conflict_branch_name(repo, name);
        repo.create_reference("refs/heads/" + newName, localTarget, false);
        // If this is the current branch, move the head to the new branch
        // so the user stays on their version of the branch.
        // We don't need to checkout as the contents will not have changed.
        if (name == current_branch_name(repo)) {
            move_head(repo, newName);
        }

        // Point the old branch to the fetched remote commits.
        repo.create_reference("refs/heads/" + name, remoteTarget, true);
        push(remote, newName, false);
        cout << "Branch " << name << " had remote changes that conflicted with yours, your commits have been moved to " << newName << ".\n";
    }

    Repository clone(const string& url, const string& path) {
        const string repoPath = path + "/.git";
        if (Repository::exists(repoPath)) {
            throw RepositoryExistsException();
        }

        Repository repo = git::Repository::clone(url, repoPath);
        // Pull all the other branches (which were fetched anyway).
        sync(repo);
        return repo;
    }

    void sync(const Repository& repo) {
        Remote origin = repo.lookup_remote("origin");
        git_fetch_options fetchOpts = GIT_FETCH_OPTIONS_INIT;
        fetchOpts.prune = GIT_FETCH_PRUNE;
        origin.fetch(StrArray(), fetchOpts);

        map<string, RefTargets> branchTargets;
        get_branch_targets(repo, &branchTargets);

        for(const auto& entry : branchTargets) {
            const string branchName = entry.first;
            const RefTargets targets = entry.second;

            if (targets.local != targets.remote) {
                OID base;
                if (!targets.local.isNull && !targets.remote.isNull) {
                    base = repo.merge_base(targets.local, targets.remote);
                }

                SyncType syncType;
                if (targets.local == targets.synced) {
                    syncType = PULL;
                } else if (targets.remote == targets.synced) {
                    syncType = PUSH;
                } else {
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
                        push(origin, branchName, targets.local.isNull);
                        break;
                    case PULL:
                        change_branch_target(repo, branchName, targets.remote);
                        break;
                    case CONFLICT:
                        create_conflict_branches(repo, origin, branchName, targets.local, targets.remote);
                        break;
                }
            }
        }

        update_sync_cache(repo);
    }

    void sync_down(const Repository& repo, bool force) {
        repo.lookup_remote("origin").fetch(StrArray(), GIT_FETCH_OPTIONS_INIT);

        const string branch = metro::current_branch_name(repo);
        MergeAnalysis analysis = metro::merge_analysis(repo, "origin/" + branch);

        if (force && (analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) == 0) {
            metro::reset_head(repo);
        } else if ((analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) == 0) {
            bool areChanged = metro::has_uncommitted_changes(repo);

            if (areChanged) {
                cout << "Cannot Sync Down with unsaved changes." << endl;
                return;
            }
        }
        if ((analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) != 0) cout << "FF" << endl;
        if ((analysis & GIT_MERGE_ANALYSIS_NONE) != 0) cout << "None?" << endl;
        if ((analysis & GIT_MERGE_ANALYSIS_NORMAL) != 0) cout << "NORMAL" << endl;
        if ((analysis & GIT_MERGE_ANALYSIS_UNBORN) != 0) cout << "Unborn" << endl;
        if ((analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) != 0) cout << "Uptodate" << endl;

        if ((analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) != 0) {
            metro::fast_forward(repo, "origin/" + branch);
            cout << "Fast-Forwarded Repo" << endl;
        } else if ((analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) == 0) {
            metro::create_branch(repo, branch + "-local");
            metro::fast_forward(repo, "origin/" + branch);

            string in;
            cout << "Conflict Found:" << endl;
            printf("[0] Absorb local %s branch into remote %s branch\n", branch.c_str(), branch.c_str());
            printf("[1] Move local changes to new branch %s-local\n", branch.c_str());
            cin >> in;
            if (in == "0" || in == "1") {
                printf("Successfully moved local changes into %s-local\n", branch.c_str());
            } else {
                printf("Invalid choice: Moved local changes into %s-local\n", branch.c_str());
            }

            try {
                metro::start_merge(repo, branch + "-local");
            } catch (UnnecessaryMergeException &e) {
                cout << "You're already in Sync." << endl;
                return;
            }

            metro::delete_branch(repo, branch + "-local");

            if (!repo.index().has_conflicts()) {
                metro::commit(repo, "Completed Absorb.", {"HEAD"});
                cout << "Successfully absorbed changes" << endl;
            } else {
                cout << "Conflicts Found: Fix, Commit and Sync again." << endl;
            }

        } else {
            cout << "You are up to date!" << endl;
        }
    }
}