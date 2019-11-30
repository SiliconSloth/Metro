#include "pch.h"

Command syncCmd {
        "sync",
        "Sync repo with remote",

        // execute
        [](const Arguments &args) {
            if (args.positionals.size() > 1) {
                throw UnexpectedPositionalException(args.positionals[1]);
            }

            if (args.positionals.empty() || args.positionals[0] == "up" || args.positionals[0] == "down") {
                Repository repo = git::Repository::open(".");

                if (args.positionals.empty()) {
                    throw UnsupportedOperationException("We don't support bidirectional syncing yet!");
                } else if (args.positionals[0] == "up") {
                    throw UnsupportedOperationException("We don't support syncing up yet!");
                } else if (args.positionals[0] == "down") {
                    metro::sync_down(repo, false);
                }
            } else {
                string url = args.positionals[0];
                // Ignore slash direction.
                replace(url.begin(), url.end(), '\\', '/');
                string name = metro::name_from_url(url);

                cout << "Syncing " << url << " into ./" << name << endl;
                metro::clone(url, name);
                cout << "Syncing complete." << endl;
            }
        },

        // printHelp
        [](const Arguments &args) {
            cout << "Usage: metro sync [up/down/url]" << endl;
        }
};