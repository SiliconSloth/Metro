#include "pch.h"

Command info {
        "info",
        "Show the state of the repo",

        // execute
        [](const Arguments &args) {
            git::Repository repo = git::Repository::open(".");
            cout << "Current branch is " << metro::current_branch_name(repo) << endl;
            cout << (metro::merge_ongoing(repo) ? "Merge ongoing" : "Not merging") << endl;
            metro::add_all(repo);
            git::Diff diff = metro::current_changes(repo);

            if (diff.num_deltas() == 0) {
                cout << "Nothing to commit" << endl;
            }
            else {
                // Print any changed files
                int added = diff.num_deltas_of_type(GIT_DELTA_ADDED);
                int deleted = diff.num_deltas_of_type(GIT_DELTA_DELETED);
                int modified = diff.num_deltas_of_type(GIT_DELTA_MODIFIED);
                int renamed = diff.num_deltas_of_type(GIT_DELTA_RENAMED);
                int copied = diff.num_deltas_of_type(GIT_DELTA_COPIED);
                if (added != 0) cout << added << " file" << (added > 1 ? "s" : "") << " to add" << endl;
                if (deleted != 0) cout << deleted << " file" << (deleted > 1 ? "s" : "") << " to delete" << endl;
                if (modified != 0) cout << modified << " file" << (modified > 1 ? "s" : "") << " to modify" << endl;
                if (renamed != 0) cout << renamed << " file" << (renamed > 1 ? "s" : "") << " to rename" << endl;
                if (copied != 0) cout << copied << " file" << (copied > 1 ? "s" : "") << " to copy" << endl;
            }
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro info\n";
        }
};
