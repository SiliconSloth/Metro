#include "pch.h"

Command syncCmd {
        "sync",
        "Sync repo with remote",

        // execute
        [](const Arguments &args) {
            if (!args.positionals.empty()) {
                throw UnexpectedPositionalException(args.positionals[0]);
            }

            Repository repo = git::Repository::open(".");
            metro::sync(repo);
        },

        // printHelp
        [](const Arguments &args) {
            cout << "Usage: metro sync" << endl;
        }
};