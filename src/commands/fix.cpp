/*
 * Defines the Fix command.
 */

enum Subcommand {
    SQUASH_WIP,
    DELETE_WIP,
    DETACH_WIP,
    ATTACH_WIP
};

/**
 * The fix command is used to fix problems with WIP and branches.
 */
Command fix {
        "fix",
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
            if (command_s == "squash") command = Subcommand::SQUASH_WIP;
            else if (command_s == "delete") command = Subcommand::DELETE_WIP;
            else if (command_s == "detach") command = Subcommand::DETACH_WIP;
            else if (command_s == "attach") command = Subcommand::ATTACH_WIP;
            else throw UnexpectedPositionalException(args.positionals[0]);

            git::Repository repo = git::Repository::open(".");
            string current_branch = metro::current_branch_name(repo);
            switch (command) {
                case SQUASH_WIP:
                    if (!metro::branch_exists(repo, metro::to_wip(current_branch))) {
                        throw AttachedWIPException();
                    }
                    break;
                case DELETE_WIP:
                    break;
                case DETACH_WIP:
                    break;
                case ATTACH_WIP:
                    break;
            }
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro fix <command>\n";
            std::cout << "\nCommands:\n";
            std::cout << " squash\tSquashes the WIP branch into a single WIP commit. Preserves changes on the WIP branch.\n";
            std::cout << " delete\tDeletes the WIP branch and any changes on it.\n";
            std::cout << " detach\tDetaches the WIP from the current branch.\n";
            std::cout << " attach\tAttaches a detached WIP to the current branch.\n";
        }
};
