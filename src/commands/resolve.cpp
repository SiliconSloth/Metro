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

            const metro::Head head = metro::get_head(repo);
            cout << "Successfully absorbed into " << head.name << ".\n";
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro resolve\n";
        }
};
