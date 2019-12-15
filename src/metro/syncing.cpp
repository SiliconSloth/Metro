#include "pch.h"

namespace metro {
    struct RefTargets {
        OID local;
        OID remote;
        OID synced;
    };

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

    bool prepare_branch_targets(map<string, RefTargets>& branchTargets, string& name, const string& prefix) {
        if (has_prefix(name, prefix)) {
            name = name.substr(prefix.size(), name.size() - prefix.size());
            if (branchTargets.find(name) == branchTargets.end()) {
                branchTargets[name] = {OID(), OID(), OID()};
            }
            return true;
        }
        return false;
    }

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

    Repository clone(const string& url, const string& path) {
        const string repoPath = path + "/.git";
        if (Repository::exists(repoPath)) {
            throw RepositoryExistsException();
        }

        Repository repo = git::Repository::clone(url, repoPath);
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

                if (targets.local == targets.synced) {
                    if (targets.local != base) {
                        if (base.isNull) {
                            delete_branch(repo, branchName);
                        } else {
                            git_checkout_options checkoutOpts = GIT_CHECKOUT_OPTIONS_INIT;
                            checkoutOpts.checkout_strategy = GIT_CHECKOUT_FORCE;
                            repo.reset_to_commit(repo.lookup_commit(base), GIT_RESET_HARD, checkoutOpts);
                        }
                    }

                    if (targets.remote != base) {
                        repo.create_reference("refs/heads/" + branchName, targets.remote, true);
                        checkout(repo, branchName);
                    }
                } else if (targets.remote == targets.synced) {
                    PushOptions opts = GIT_PUSH_OPTIONS_INIT;
                    opts.callbacks.credentials = acquire_credentials;
                    string refspec = targets.local.isNull? ":refs/heads/" + branchName : "+refs/heads/" + branchName + ":refs/heads/" + branchName;
                    StrArray refspecs({refspec});
                    origin.push(refspecs, opts);
                    cout << "Pushed to remote " << entry.first << "." << endl;
                } else {
                    cout << "Need to resolve conflict on " << entry.first << "." << endl;
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