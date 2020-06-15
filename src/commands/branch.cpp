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

            // Finds differences between head and working dir
            git::Tree current = metro::get_commit(repo, "HEAD").tree();
            git_diff_options opts = GIT_DIFF_OPTIONS_INIT;
            git::Diff diff = git::Diff::tree_to_workdir(repo, current, &opts);

            if (diff.num_deltas() > 0) {
                cout << "Saved changes to WIP" << endl;
            }

            metro::switch_branch(repo, name);
            const metro::Head head = metro::get_head(repo);
            cout << "Switched to branch " << head.name << ".\n";
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro branch <name>\n";
        }
};
