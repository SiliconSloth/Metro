/*
 * Defines the Absorb command.
 */

/**
 * The absorb command is used to merge one branch into another, similar to `git merge`
 */
Command absorbCmd {
        "absorb",
        "Merge the changes in another branch into this one",

        // execute
        [](const Arguments &args) {
            if (args.positionals.empty()) {
                throw MissingPositionalException("other-branch");
            }
            if (args.positionals.size() > 1) {
                throw UnexpectedPositionalException(args.positionals[1]);
            }
            string name = args.positionals[0];

            git::Repository repo = git::Repository::open(".");
            if (repo.head_detached()) {
                throw UnsupportedOperationException("You must be on a branch to absorb.");
            }

            bool hasConflicts = metro::absorb(repo, name);
            if (hasConflicts) {
                cout << "Conflicts occurred, please resolve." << endl;
            } else {
                const metro::Head head = metro::get_head(repo);
                cout << "Successfully absorbed " << name << " into " << head.name << ".\n";
            }
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro absorb <other-branch>\n";
        }
};
