#include "pch.h"

namespace metro {
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

    Repository clone(const string& url, const string& path) {
        const string repoPath = path + "/.git";
        if (Repository::exists(repoPath)) {
            throw RepositoryExistsException();
        }

        Repository repo = git::Repository::clone(url, repoPath);
        return repo;
    }

    void sync_down(const Repository& repo, bool force) {
        Remote remote = repo.lookup_remote("origin");
        string branch = metro::current_branch_name(repo);

        FetchOps fetchOps = GIT_FETCH_OPTIONS_INIT;
        metro::create_callbacks(&fetchOps.callbacks);

        metro::remote_fetch(remote, StrArray(), fetchOps, "pull");

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