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

            bool force = args.options.find("force") != args.options.end();
            bool saveWip = true;

            git::Repository repo = git::Repository::open(".");

            if (repo.head_detached() && metro::has_uncommitted_changes(repo)) {
                if (force) {
                    saveWip = false;
                    cout << "Discarding uncommitted changes on current branch." << endl;
                } else {
                    throw MetroException("Your uncommitted changes cannot be saved as you are not on a branch.\n"
                                         "If you would like to switch anyway, use --force");
                }
            }

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

            metro::switch_branch(repo, name, saveWip);
            cout << "Switched to branch " << name << ".\n";
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro switch <branch>\n";
            print_options({"force", "help"});
        }
};
