#include "pch.h"

Command create {
        "create",
        "Create a repo",

        // execute
        [](const Arguments &args) {
            string directory = ".";
            if (!args.positionals.empty()) {
                directory = args.positionals[0];
            }
            if (args.positionals.size() > 1) {
                throw UnexpectedPositionalException(args.positionals[1]);
            }

            metro::create(directory);
            cout << "Created Metro repo.\n";
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro create [directory]\n";
        }
};
