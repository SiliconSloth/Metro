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

    void get_keys(string *pub, string *pri) {
#ifdef _WIN32
        cout << "Metro currently doesn't support SSH on Windows. Please use HTTPS." << endl;
        return;
#elif __unix__
        const char* home;
        home = getenv("HOME");

        if (home == NULL) {
            home = getpwuid(getuid())->pw_dir;
        }

        ifstream keyfile;
        keyfile.open(string(home) + "/.ssh/id_rsa.pub");
        if (keyfile.is_open()) {
            string line;
            while (getline(keyfile, line)) {
                pub->append(line);
            }
            keyfile.close();
        } else {
            cout << "Public Key not found at " + string(home) + "/.ssh/id_rsa.pub" << endl;
        }
        keyfile.open(string(home) + "/.ssh/id_rsa");
        if (keyfile.is_open()) {
            string line;
            while (getline(keyfile, line)) {
                pri->append(line);
            }
            keyfile.close();
        } else {
            cout << "Public Key not found at " + string(home) + "/.ssh/id_rsa" << endl;
        }
#endif //_WIN32
    }

    int acquire_credentials(git_cred **cred, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload) {
        int err = GIT_OK;
        auto credentials = static_cast<git_cred**>(payload);

        if (*credentials == nullptr) {
            string username;
            string password;
            switch (allowed_types) {
                case GIT_CREDTYPE_DEFAULT:
                    err = git_cred_default_new(credentials);
                    break;
                case GIT_CREDTYPE_USERPASS_PLAINTEXT:
                    cout << "Username for " << url << ": ";
                    getline(cin, username);
                    cout << "Password for " << username << ": ";
                    password = read_password();

                    err = git_cred_userpass_plaintext_new(credentials, username.c_str(), password.c_str());
                    break;
                default:
                    cout << "Username for " << url << ": ";
                    getline(cin, username);
                    cout << "SSH Keystore Password: ";
                    password = read_password();

                    string pub, pri;
                    get_keys(&pub, &pri);
//                cout << "Metro currently doesn't support SSH. Please use HTTPS." << endl;
                    cout << "Public key is:\n" << pub << endl;
                    cout << "Private key is:\n" << pri << endl;
                    err = git_cred_ssh_key_new(credentials, username.c_str(), pub.c_str(), pri.c_str(), password.c_str());
                    break;
            }
        }

        *cred = *credentials;
        return err;
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

    int transfer_progress(const git_indexer_progress* stats, void* payload) {
        int progress = (100 * (stats->received_objects + stats->indexed_objects)) / (2 * stats->total_objects);
        string bar;
        int i;
        for (i = 0; i < progress; i++) {
            bar.append("=");
        }
        for (; i < 100; i++) {
            bar.append("-");
        }
        printf("\rProgress: [%s] %d%%", bar.c_str(), progress);
        return GIT_OK;
    }


    Repository clone(const string& url, const string& path) {
        git_cred *credentials = nullptr;
        Repository repo = clone(url, path, &credentials);
        if (credentials != nullptr) {
            git_cred_free(credentials);
        }
        return repo;
    }

    Repository clone(const string& url, const string& path, git_cred** credentials) {
        const string repoPath = path + "/.git";
        if (Repository::exists(repoPath)) {
            throw RepositoryExistsException();
        }

        git_clone_options options = GIT_CLONE_OPTIONS_INIT;
        options.fetch_opts.callbacks.credentials = acquire_credentials;
        options.fetch_opts.callbacks.payload = credentials;
        options.fetch_opts.callbacks.transfer_progress = transfer_progress;
        Repository repo = git::Repository::clone(url, repoPath, &options);
        // Pull all the other branches (which were fetched anyway).
        sync(repo, credentials);
        return repo;
    }

    void sync(const Repository& repo) {
        git_cred *credentials = nullptr;
        sync(repo, &credentials);
        if (credentials != nullptr) {
            git_cred_free(credentials);
        }
    }

    void sync(const Repository& repo, git_cred** credentials) {
        save_wip(repo);

        Remote origin = repo.lookup_remote("origin");
        git_fetch_options fetchOpts = GIT_FETCH_OPTIONS_INIT;
        fetchOpts.prune = GIT_FETCH_PRUNE;
        fetchOpts.callbacks.credentials = acquire_credentials;
        fetchOpts.callbacks.payload = credentials;
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
                        pushRefspecs.push_back(make_push_refspec(branchName, targets.local.isNull));
                        break;
                    case PULL:
                        change_branch_target(repo, branchName, targets.remote);
                        break;
                    case CONFLICT:
                        create_conflict_branches(repo, origin, branchName, targets.local, targets.remote,
                                branchTargets, conflictBranchNames, pushRefspecs);
                        break;
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
                create_conflict_branches(repo, origin, oldName, target, target,
                        branchTargets, conflictBranchNames, pushRefspecs);
            }
        }

        if (!pushRefspecs.empty()) {
            PushOptions options = GIT_PUSH_OPTIONS_INIT;
            options.callbacks.credentials = acquire_credentials;
            options.callbacks.payload = credentials;
            origin.push(StrArray(pushRefspecs), options);
        }

        update_sync_cache(repo);
        restore_wip(repo);
    }
}