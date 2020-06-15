/*
 * Defines the Switch command.
 */

/**
 * The switch command switches from the current repo to the target repo, saving work to WIP and loading new work from
 * WIP if any.
 */
Command switchCmd {
        "switch",
        "Switch to a different branch",

        // execute
        [](const Arguments &args) {
            if (args.positionals.empty()) {
                throw MissingPositionalException("branch");
            }
            if (args.positionals.size() > 1) {
                throw UnexpectedPositionalException(args.positionals[1]);
            }
            string name = args.positionals[0];

            git::Repository repo = git::Repository::open(".");

            string wip = metro::to_wip(name);
            bool exists = metro::branch_exists(repo, wip);

            // If branch is current branch
            if (metro::is_on_branch(repo, name)) {
                if (exists) {
                    metro::restore_wip(repo);
                    cout << "Loaded changes from WIP" << endl;
                } else {
                    cout << "You are already on branch " << name << endl;
                }
                return;
            }

            // Finds differences between head and working dir
            git::Tree current = metro::get_commit(repo, "HEAD").tree();
            git_diff_options opts = GIT_DIFF_OPTIONS_INIT;
            git::Diff diff = git::Diff::tree_to_workdir(repo, current, &opts);

            if (diff.num_deltas() > 0) {
                cout << "Saved changes to WIP" << endl;
            }

            metro::switch_branch(repo, name);
            cout << "Switched to branch " << name << ".\n";

            if (exists) {
                cout << "Loaded changes from WIP" << endl;
            }
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro switch <branch>\n";
        }
};
