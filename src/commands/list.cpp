#include "pch.h"

Command listCmd{
        "list",
        "Lists the commits or branches",

        // execute
        [](const Arguments& args) {
            if (args.positionals.empty()) {
                throw MissingPositionalException("type");
            }

            Repository repo = Repository::open(".");
            metro::assert_merging(repo);

            if (args.positionals[0] == "commit") {
                if (args.positionals.size() > 1) {
                    throw UnexpectedPositionalException(args.positionals[1]);
                }
                Commit commit = metro::get_commit(repo, "HEAD");
                unsigned int count = commit.parentcount();
                for (unsigned int i = 0; i < count; i++) {
                    OID nth_parent_id = commit.parentID(i);

                    Commit nth_parent = commit.parent(i);

                    cout << nth_parent.message() << endl;
                }
            } else if (args.positionals[0] == "branch") {
                if (args.positionals.size() > 1) {
                    throw UnexpectedPositionalException(args.positionals[1]);
                }

                BranchIterator iter = repo.new_branch_iterator(GIT_BRANCH_LOCAL);
                for (Branch branch; iter.next(&branch);) {
                    cout << branch.name() << endl;
                }
            } else {
                throw UnexpectedPositionalException(args.positionals[0]);
            }
    },

    // printHelp
    [](const Arguments& args) {
        if (args.positionals.empty() || (args.positionals[0] != "commit" && args.positionals[0] != "branch")) {
            cout << "Usage: metro delete <commit/branch>\n";
        }
        if (!args.positionals.empty()) {
            if (args.positionals[0] == "commit") {
                cout << "Usage: metro delete commit\n";
            }
            if (args.positionals[0] == "line") {
                cout << "Usage: metro delete branch <branch>";
            }
        }
    }
};

