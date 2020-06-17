/*
 * Defines the Wip command.
 */

enum Subcommand {
    SAVE_WIP,
    DELETE_WIP,
    RESTORE_WIP,
    SQUASH_WIP
};

/**
 * The wip command is used to fix problems with WIP and branches.
 */
Command wip {
        "wip",
        "Fix issues with WIP",

        // execute
        [](const Arguments &args) {
            if (args.positionals.empty()) {
                throw MissingPositionalException("Sub-command");
            }
            if (args.positionals.size() > 1) {
                throw UnexpectedPositionalException(args.positionals[1]);
            }
            string command_s = args.positionals[0];
            Subcommand command;
            if (command_s == "save") command = Subcommand::SAVE_WIP;
            else if (command_s == "delete") command = Subcommand::DELETE_WIP;
            else if (command_s == "restore") command = Subcommand::RESTORE_WIP;
            else if (command_s == "squash") command = Subcommand::SQUASH_WIP;
            else throw UnexpectedPositionalException(args.positionals[0]);

            git::Repository repo = git::Repository::open(".");
            metro::Head current_branch = metro::get_head(repo);
            if (current_branch.detached) {
                throw MetroException("'metro wip' can only be used on a branch.");
            }
            if (command != SAVE_WIP && !metro::branch_exists(repo, metro::to_wip(current_branch.name))) {
                throw AttachedWIPException();
            }
            switch (command) {
                case SAVE_WIP:
                    break;
                case DELETE_WIP:
                    break;
                case RESTORE_WIP:
                    break;
                case SQUASH_WIP:
                    break;
            }
        },

        // printHelp
        [](const Arguments &args) {
            string command_s = !args.positionals.empty() ? args.positionals[0] : "";
            string save_message = "Saves the contents of the working directory to a WIP commit in a #wip branch.";
            string delete_message = "Deletes the WIP branch and any changes on it.";
            string restore_message = "Restores the WIP branch deleting any changes in the working directory.";
            string squash_message = "Squashes the WIP branch into a single WIP commit.";
            if (command_s == "save") {
                cout << save_message << endl;
                cout << "Usage: metro wip save\n";
            } else if (command_s == "delete") {
                cout << delete_message << endl;
                cout << "Usage: metro wip delete\n";
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
                cout << " delete\t\t" << delete_message << "\n";
                cout << " restore\t" << restore_message << "\n";
                cout << " squash\t\t" << squash_message << "\n";
            }
        }
};
