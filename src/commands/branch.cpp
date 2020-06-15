/*
 * Defines the Branch command.
 */

/**
 * The branch command is used to create new branches.
 */
Command branch {
        "branch",
        "Create a new branch",

        // execute
        [](const Arguments &args) {
            if (args.positionals.empty()) {
                throw MissingPositionalException("Branch name");
            }
            if (args.positionals.size() > 1) {
                throw UnexpectedPositionalException(args.positionals[1]);
            }
            string name = args.positionals[0];

            if (metro::is_wip(name)) {
                throw MetroException("Branch name can't end in " + string(WIP_SUFFIX));
            }

            git::Repository repo = git::Repository::open(".");
            if (metro::branch_exists(repo, name)) {
                throw MetroException("Branch " + name + " already exists.");
            }

            if (!metro::head_exists(repo)) {
                throw UnsupportedOperationException("Cannot create branch when the current branch is empty.\n"
                                                    "Please make an initial commit first.");
            }

            metro::create_branch(repo, name);
            cout << "Created branch " + name + "." << endl;

            if (repo.head_detached() && metro::has_uncommitted_changes(repo)) {
                cout << "Could not switch to new branch due to uncommitted changes." << endl;
            } else {
                metro::switch_branch(repo, name);
                const metro::Head head = metro::get_head(repo);
                cout << "Switched to branch " << head.name << ".\n";
            }
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro branch <name>\n";
        }
};
