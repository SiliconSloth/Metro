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
            StrArray remotes = repo.remote_list();

            if (remotes.count() < 1) {
                cout << "What url should be fetched from?" << endl;
                string url;
                cin >> url;

                // add remote
            } else {
                // Lookup first remote
            }
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro sync <up/down/url>\n";
        }
};
