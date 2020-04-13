/*
 * Defines the Resolve command.
 */

/**
 * The resolve command is used to resolve conflicts after a merge.
 */
Command resolve {
        "resolve",
        "Commit resolved conflicts after absorb",

        // execute
        [](const Arguments &args) {
            git::Repository repo = git::Repository::open(".");
            metro::resolve(repo);

            string current = metro::current_branch_name(repo);
            cout << "Successfully absorbed into " << current << ".\n";
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro resolve\n";
        }
};
