/*
 * Defines the Wip command.
 */

enum Subcommand {
    SAVE_WIP,
    RESTORE_WIP,
    SQUASH_WIP
};

/**
 * The wip command is used to fix problems with WIP and branches.
 */
Command wip {
        "wip",
        "Fix issues with " WIP_SUFFIX " branches",

        // execute
        [](const Arguments &args) {
            if (args.positionals.empty()) {
                throw MissingPositionalException("command");
            }
            if (args.positionals.size() > 1) {
                throw UnexpectedPositionalException(args.positionals[1]);
            }
            string command_s = args.positionals[0];
            Subcommand command;
            if (command_s == "save") command = Subcommand::SAVE_WIP;
            else if (command_s == "restore") command = Subcommand::RESTORE_WIP;
            else if (command_s == "squash") command = Subcommand::SQUASH_WIP;
            else throw UnexpectedPositionalException(args.positionals[0]);

            git::Repository repo = git::Repository::open(".");
            metro::Head current_branch = metro::get_head(repo);
            if (current_branch.detached) {
                throw MetroException("'metro wip' can only be used on a branch.");
            }
            string wipBranch = metro::to_wip(current_branch.name);
            if (command == SAVE_WIP && !metro::has_uncommitted_changes(repo)) {
                cout << "No uncommitted changes to save." << endl;
                return;
            }
            if (command != SAVE_WIP && !metro::branch_exists(repo, wipBranch)) {
                throw NeedWIPException();
            }
            if (command == SAVE_WIP && metro::branch_exists(repo, wipBranch)) {
                throw UnexpectedWIPException(wipBranch);
            }

            switch (command) {
                case SAVE_WIP:
                    metro::save_wip(repo);
                    cout << "Saved changes to " << wipBranch << "." << endl;
                    cout << "Removed changes from the working directory." << endl;
                    break;
                case RESTORE_WIP:
                    metro::restore_wip(repo, args.options.find("force") != args.options.end());
                    cout << "Restored changes from " << wipBranch << endl;
                    break;
                case SQUASH_WIP:
                    metro::squash_wip(repo, args.options.find("force") != args.options.end());
                    cout << "Squashed " << wipBranch << " into a single commit." << endl;
                    cout << "Use `metro wip restore` to restore the changes from " << wipBranch << endl;
                    break;
            }
        },

        // printHelp
        [](const Arguments &args) {
            string command_s = !args.positionals.empty() ? args.positionals[0] : "";
            string save_message = "Saves the contents of the working directory to a WIP commit in a " WIP_SUFFIX " branch.";
            string restore_message = "Restores the " WIP_SUFFIX " branch, deleting any changes in the working directory.";
            string squash_message = "Squashes the " WIP_SUFFIX " branch, into a single WIP commit.";
            if (command_s == "save") {
                cout << save_message << endl;
                cout << "Usage: metro wip save\n";
            } else if (command_s == "restore") {
                cout << restore_message << endl;
                cout << "Usage: metro wip restore\n";
            } else if (command_s == "squash") {
                cout << squash_message << endl;
                cout << "Usage: metro wip squash\n";
            } else {
                cout << "Usage: metro wip <command>\n";
                cout << "\nCommands:\n";
                cout << " save\t\t" << save_message << "\n";
                cout << " restore\t" << restore_message << "\n";
                cout << " squash\t\t" << squash_message << "\n";
            }
        }
};
