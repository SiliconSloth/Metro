#include "pch.h"

Command patch {
        "patch",
        "Update the last commit with the current work",

        // execute
        [](const Arguments &args) {
            Repository repo = git::Repository::open(".");
            metro::assert_not_merging(repo);

            // Uses existing message as default
            Commit commit = static_cast<Commit>(repo.revparse_single("HEAD"));
            string message = commit.message();

            if (args.positionals.size() == 1) {
                message = args.positionals[0];
            }
            if (args.positionals.size() > 1) {
                throw UnexpectedPositionalException(args.positionals[1]);
            }

            metro::patch(repo, message);
            cout << "Patched commit.\n";
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro patch <message>\n";
        }
};
