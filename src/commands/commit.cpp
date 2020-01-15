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
                metro::commit(repo, message, { "HEAD" });
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
