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

            if (!git_reference_is_valid_name(("refs/heads/" + name).c_str())) {
                throw MetroException("Branch name '" + name + "' is an invalid name");
            }

            git::Repository repo = git::Repository::open(".");
            if (metro::branch_exists(repo, name)) {
                throw MetroException("Branch " + name + " already exists.");
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
            cout << "Switched to branch " << name << ".\n";
            cout << "Currently on branch " + metro::current_branch_name(repo) << endl;
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro branch <name>\n";
        }
};
