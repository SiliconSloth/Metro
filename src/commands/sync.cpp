#include "pch.h"

void downsync(const Repository &repo, Remote remote, bool force) {
    string check = "curl -Is " + string(git_remote_url(remote)) + " > /dev/null";
    if (system(check.c_str())) {
        cout << "Could not connect to " << git_remote_url(remote) << endl;
        return;
    }

    string branch = metro::current_branch_name(repo);

    FetchOps fetchOps = GIT_FETCH_OPTIONS_INIT;
    metro::create_callbacks(&fetchOps.callbacks);

    metro::remote_fetch(remote, StrArray(), fetchOps, "pull");

    MergeAnalysis analysis = metro::merge_analysis(repo, "origin/master");

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

Command syncCmd {
        "sync",
        "Sync repo with remote",

        // execute
        [](const Arguments &args) {
            if (!args.positionals.empty()) {
                throw UnexpectedPositionalException(args.positionals[0]);
            }

            Repository repo = git::Repository::open(".");
            StrArray remotes = repo.remote_list();

            Remote remote;
            if (remotes.count() < 1) {
                cout << "What url should be fetched from?" << endl;
                string url;
                cin >> url;

                remote = metro::add_remote(repo, url);
            } else {
                remote = repo.remote_lookup(remotes.strings()[0]);
            }

            downsync(repo, remote, false);

            remotes.free();
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro sync <up/down/url>\n";
        }
};