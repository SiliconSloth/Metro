/*
 * Defines the Patch command.
 */

/**
 * The patch command is used to patch the last commit.
 */
Command patch {
        "patch",
        "Update the last commit with the current work",

        // execute
        [](const Arguments &args) {
            git::Repository repo = git::Repository::open(".");
            if (!metro::head_exists(repo)) {
                throw MetroException("No commit to patch.");
            }
            metro::assert_not_merging(repo);

            // Uses existing message as default
            git::Commit commit = static_cast<git::Commit>(repo.revparse_single("HEAD"));
            string message = commit.message();

            if (args.positionals.size() == 1) {
                message = args.positionals[0];
            }
            if (args.positionals.size() > 1) {
                throw UnexpectedPositionalException(args.positionals[1]);
            }

            // Check owner if force not passed
            if (args.options.find("force") == args.options.end()) {
                string commit_auth((const char *) commit.author().name);
                string current_auth((const char *) repo.default_signature().name);
                string commit_auth_e((const char *) commit.author().email);
                string current_auth_e((const char *) repo.default_signature().email);
                if (commit_auth != current_auth && commit_auth_e != current_auth_e) {
                    cout << "Your credentials are different to the author of the commit you are trying to patch." << endl;
                    cout << "Patching the commit will override their credentials with your own." << endl;
                    cout << "If you would still like to patch, use metro patch --force." << endl;
                    return;
                }
            }

            metro::patch(repo, message);
            cout << "Patched commit.\n";
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro patch [message]\n";
        }
};
