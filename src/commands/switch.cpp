#include "pch.h"

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

            Repository repo = git::Repository::open(".");

            string wip = metro::to_wip(name);
            bool exists = metro::branch_exists(repo, wip);

            // Finds differences between head and working dir
            Tree current = metro::get_commit(repo, "HEAD").tree();
            DiffOptions opts = GIT_DIFF_OPTIONS_INIT;
            Diff diff = Diff::tree_to_workdir(repo, current, &opts);

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
