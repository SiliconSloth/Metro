/*
 * Defines the Rename command.
 */

/**
 * The rename command is used to rename a branch.
 */
Command renameCmd {
        "rename",
        "Rename a branch",

        // execute
        [](const Arguments &args) {
            if (args.positionals.empty()) {
                throw MissingPositionalException("branch-1");
            }
            if (args.positionals.size() > 2) {
                throw UnexpectedPositionalException(args.positionals[2]);
            }

            git::Repository repo = git::Repository::open(".");

            string from, to;
            if (args.positionals.size() == 1) {
                to = args.positionals[0];
                try {
                    from = metro::current_branch_name(repo);
                } catch (BranchNotFoundException &e) {
                    throw MetroException("The HEAD is not pointing at any branch, so cannot rename.\nTry using 'metro rename <branch> " + to + "'.");
                }
            } else {
                from = args.positionals[0];
                to = args.positionals[1];
            }

            bool force = args.options.find("force") != args.options.end();

            // Ensure target branch + wip doesn't exist
            if (metro::branch_exists(repo, to) && !force) throw UnsupportedOperationException("There is already a branch with that name.\nTo overwrite it, use 'metro rename --force'.");
            if (metro::branch_exists(repo, metro::to_wip(to)) && !force) throw UnsupportedOperationException("There is a WIP branch for the target branch name.\nTo overwrite it, use 'metro rename --force'.");

            git::Branch current = repo.lookup_branch(from, GIT_BRANCH_LOCAL);
            current.rename(to, force);

            // Delete target wip if exists
            if (metro::branch_exists(repo, metro::to_wip(to))) {
                metro::delete_branch(repo, metro::to_wip(to));
            }

            // Move wip to target wip
            if (metro::branch_exists(repo, metro::to_wip(from))) {
                git::Branch wip = repo.lookup_branch(metro::to_wip(from), GIT_BRANCH_LOCAL);
                wip.rename(metro::to_wip(to), force);
            }
            cout << "Renamed branch " << from << " to " << to << "." << endl;
        },

        // printHelp
        [](const Arguments &args) {
            cout << "Usage: metro rename <branch-1> [<branch-2>]" << endl;
        }
};