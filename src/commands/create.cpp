#include "pch.h"

Command create {
        "create",
        "Create a repo",

        // execute
        [](const Arguments &args) {
            std::cout << "Let's create " << args.positionals[0] << "\n";
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro create [directory]\n";
        }
};
