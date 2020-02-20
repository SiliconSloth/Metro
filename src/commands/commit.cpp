#include "pch.h"

Command commit {
        "commit",
        "Make a commit",

        // execute
        [](const Arguments &args) {
            if (args.positionals.empty()) {
                throw MissingPositionalException("message");
            }
            if (args.positionals.size() > 1) {
                throw UnexpectedPositionalException(args.positionals[1]);
            }
            string message = args.positionals[0];

            Repository repo = git::Repository::open(".");
            metro::assert_merging(repo);

            try {
                metro::add_all(repo);
                Diff diff = metro::current_changes(repo);

                // If no changes, exit
                if (diff.num_deltas() == 0) {
                    throw UnsupportedOperationException("No files to commit");
                }

                metro::commit(repo, message, { "HEAD" });

                // Deletes WIP branch if any
                string name = metro::current_branch_name(repo);
                string wipName = metro::to_wip(name);
                if (metro::branch_exists(repo, wipName)) {
                    metro::delete_branch(repo, wipName);
                }

                // Print any changed files
                int added = diff.num_deltas_of_type(GIT_DELTA_ADDED);
                int deleted = diff.num_deltas_of_type(GIT_DELTA_DELETED);
                int modified = diff.num_deltas_of_type(GIT_DELTA_MODIFIED);
                int renamed = diff.num_deltas_of_type(GIT_DELTA_RENAMED);
                int copied = diff.num_deltas_of_type(GIT_DELTA_COPIED);
                if (added != 0) cout << added << " file" << (added > 1 ? "s" : "") << " added" << endl;
                if (deleted != 0) cout << deleted << " file" << (deleted > 1 ? "s" : "") << " deleted" << endl;
                if (modified != 0) cout << modified << " file" << (modified > 1 ? "s" : "") << " modified" << endl;
                if (renamed != 0) cout << renamed << " file" << (renamed > 1 ? "s" : "") << " renamed" << endl;
                if (copied != 0) cout << copied << " file" << (copied > 1 ? "s" : "") << " copied" << endl;

                string branch = metro::current_branch_name(repo);
                cout << "Saved commit to branch " << branch << "." << endl;
            } catch (UnsupportedOperationException &e) {
                cout << e.what() << endl;
            }
        },

        // printHelp
        [](const Arguments &args) {
            cout << "Usage: metro commit <message>" << endl;
        }
};
