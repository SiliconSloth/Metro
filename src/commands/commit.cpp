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
            metro::assertMerging(repo);

            metro::commit(repo, message, {"HEAD"});
            cout << "Saved commit to current branch.\n";
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro commit <message>\n";
        }
};
