#include "pch.h"

Command clone_repo {
        "clone",
        "Clone a remote repo",

        // execute
        [](const Arguments &args) {
            if (args.positionals.empty()) {
                throw MissingPositionalException("url");
            }
            if (args.positionals.size() > 1) {
                throw UnexpectedPositionalException(args.positionals[1]);
            }

            string url = args.positionals[0];
            // Ignore slash direction.
            replace(url.begin(), url.end(), '\\', '/');
            string name = metro::UrlDescriptor(url).repository;
            if (name.empty()) {
                throw UnsupportedOperationException("Couldn't find repository name in URL.");
            }

            exit_config.cloning = true;
            exit_config.directory = name;

            cout << "Cloning " << url << " into " << name << endl;
            metro::clone(url, name);
            cout << "Cloning complete." << endl;
        },

        // printHelp
        [](const Arguments &args) {
            cout << "Usage: metro clone <url>" << endl;
        }
};