/*
 * Defines the Switch command.
 */

/**
 * The switch command switches from the current branch/revision to the branch/revision repo,
 * saving work to WIP and loading new work from WIP if any.
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
                    metro::restore_wip(repo, false);
                    cout << "Loaded changes from WIP" << endl;
                } else {
                    cout << "You are already on branch " << name << endl;
                }
                return;
            }

            metro::switch_branch(repo, name, saveWip, true);

            git::OID head = metro::get_commit(repo, "HEAD").id();
            if (repo.head_detached()) {
                cout << "Switched to commit " << head.str() << endl;
                cout << "Head is currently detached.  You will not be able to make new commits "
                        "until you switch back to a branch." << endl;
            } else {
                cout << "Switched to branch " << name << ".\n";
            }

            git::BranchIterator iter = repo.new_branch_iterator(GIT_BRANCH_LOCAL);
            for (git::Branch branch; iter.next(&branch);) {
                if (branch.target() == head && metro::is_wip(branch.name())) {
                    cout << "WARNING: You are currently on a WIP branch (" << branch.name() << ")" << endl;
                    cout << "Making changes to a WIP branch can corrupt your repository, "
                            "so it is highly recommended that you switch to a normal branch!" << endl;
                }
            }
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro switch <branch>\n";
            print_options({"force", "help"});
        }
};
