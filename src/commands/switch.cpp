#include "pch.h"

Command switchCmd {
        "switch",
        "Switch to a different branch",

        // execute
        [](const Arguments &args) {
            if (args.positionals.empty()) {
                throw MissingPositionalException("branch");
            }
            if (args.positionals.size() > 1) {
                throw UnexpectedPositionalException(args.positionals[1]);
            }
            string name = args.positionals[0];

            Repository repo = git::Repository::open(".");
            metro::switch_branch(repo, name);
            cout << "Switched to branch " << name << ".\n";
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro switch <branch>\n";
        }
};
