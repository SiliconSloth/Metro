#include "pch.h"

Command branch {
        "branch",
        "Create a new branch",

        // execute
        [](const Arguments &args) {
            if (args.positionals.empty()) {
                throw MissingPositionalException("Branch name");
            }
            if (args.positionals.size() > 1) {
                throw UnexpectedPositionalException(args.positionals[1]);
            }
            string name = args.positionals[0];

            if (has_suffix(name, WIPString)) {
                throw MetroException("Branch name can't end in " + string(WIPString));
            }

            Repository repo = Repository::open(".");
            if (metro::branch_exists(repo, name)) {
                throw MetroException("Branch " + name + " already exists.");
            }

            metro::create_branch(repo, name);
            cout << "Created branch " + name + "." << endl;
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro branch <name>\n";
        }
};
