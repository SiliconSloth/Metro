#include "pch.h"

Command syncCmd {
        "sync",
        "Sync repo with remote",

        // execute
        [](const Arguments &args) {
            if (!args.positionals.empty()) {
                throw UnexpectedPositionalException(args.positionals[0]);
            }

            metro::SyncDirection direction = metro::BOTH;
            if (args.options.find("push") != args.options.end()) {
                if (args.options.find("pull") != args.options.end()) {
                    throw InvalidOptionException("pull", "push");
                }
                direction = metro::UP;
            } else if (args.options.find("pull") != args.options.end()) {
                direction = metro::DOWN;
            }

            Repository repo = git::Repository::open(".");
            metro::sync(repo, direction, false);
        },

        // printHelp
        [](const Arguments &args) {
            cout << "Usage: metro sync" << endl;
        }
};