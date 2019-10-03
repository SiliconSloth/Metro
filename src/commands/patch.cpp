#include "pch.h"

Command patch {
        "patch",
        "Update the last commit with the current work",

        // execute
        [](const Arguments &args) {
            Repository repo = git::Repository::open(".");

            // Uses existing message as default
            Commit commit = reinterpret_cast<Commit>(repo.revparse_single("HEAD"));
            string message = commit.

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
            std::cout << "Usage: metro patch <message>\n";
        }
};
