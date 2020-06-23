/*
 * Defines the Delete command.
 */

/**
 * The delete command is used to delete a branch by name.
 */
Command deleteCmd {
        "delete",
        "Delete a branch",

        // execute
        [](const Arguments &args) {
            if (args.positionals.empty()){
                throw MissingPositionalException("branch");
            }
            if (args.positionals.size() > 1) {
                throw UnexpectedPositionalException(args.positionals[1]);
            }

            git::Repository repo = git::Repository::open(".");
            metro::assert_not_merging(repo);

            string name = args.positionals[0];
            metro::delete_branch(repo, name);
            cout << "Deleted branch " << name << ".\n";
        },

        // printHelp
        [](const Arguments &args) {
            cout << "Usage: metro delete <branch>\n";
            print_options({"help"});
        }
};
