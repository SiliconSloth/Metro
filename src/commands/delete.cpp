#include "pch.h"

Command deleteCmd {
        "delete",
        "Deletes a commit or branch",

        // execute
        [](const Arguments &args) {
            if (args.positionals.empty()){
                throw MissingPositionalException("type");
            }

            Repository repo = Repository::open(".");
            metro::assert_merging(repo);

            if (args.positionals[0] == "commit") {
                if (args.positionals.size() > 1) {
                    throw UnexpectedPositionalException(args.positionals[1]);
                }
                metro::delete_last_commit(repo, false);
                cout << "Deleted last commit.\n";
            } else if (args.positionals[0] == "branch") {
                if (args.positionals.size() < 2) {
                    throw MissingPositionalException("branch");
                }
                if (args.positionals.size() > 2) {
                    throw UnexpectedPositionalException(args.positionals[2]);
                }

                string name = args.positionals[1];
                if (name == metro::current_branch_name(repo)) {
                    throw UnsupportedOperationException("Can't delete current branch.");
                }

                metro::delete_branch(repo, name);
                cout << "Deleted branch " << name << ".\n";
            } else {
                throw UnexpectedPositionalException(args.positionals[0]);
            }
        },

        // printHelp
        [](const Arguments &args) {
            if (args.positionals.empty() || (args.positionals[0] != "commit" && args.positionals[0] != "branch")) {
                cout << "Usage: metro delete <commit/branch>\n";
            }
            if (!args.positionals.empty()) {
                if (args.positionals[0] == "commit") {
                    cout << "Usage: metro delete commit\n";
                }
                if (args.positionals[0] == "line") {
                    cout << "Usage: metro delete branch <branch>";
                }
            }
        }
};
