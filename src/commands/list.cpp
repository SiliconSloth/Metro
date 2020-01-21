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

            if (args.positionals[0] == "commits") {
                if (args.positionals.size() > 1) {
                    throw UnexpectedPositionalException(args.positionals[1]);
                }
                Commit commit = metro::get_commit(repo, "HEAD");
                unsigned int count = commit.parentcount();

                void* hConsole;
#ifdef _WIN32
                hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif // _WIN32
                for (unsigned int i = 0; i < count; i++) {
                    OID nth_parent_id = commit.parentID(i);
                    Commit nth_parent = commit.parent(i);

                    set_text_colour("rg------", hConsole);
                    cout << "Commit " << nth_parent.id().str() << endl;
                    set_text_colour("rgb-----", hConsole);

                    Signature author = nth_parent.author();
                    cout << "Author: " << author.name << " (" << author.email << ")" << endl;
                    cout << "Date: " << time_to_string(author.when) << endl;
                    cout << "\n    " << nth_parent.message() << endl;

                    if (i != count - 1) cout << endl;
                }
            } else if (args.positionals[0] == "branches") {
                if (args.positionals.size() > 1) {
                    throw UnexpectedPositionalException(args.positionals[1]);
                }

                BranchIterator iter = repo.new_branch_iterator(GIT_BRANCH_LOCAL);
                cout << "Branches:" << endl;
                for (Branch branch; iter.next(&branch);) {
                    cout << " - " << branch.name() << endl;
                }
            } else {
                throw UnexpectedPositionalException(args.positionals[0]);
            }
    },

    // printHelp
    [](const Arguments& args) {
        if (args.positionals.empty() || (args.positionals[0] != "commits" && args.positionals[0] != "branches")) {
            cout << "Usage: metro list <commits/branches>\n";
        }
        if (!args.positionals.empty()) {
            if (args.positionals[0] == "commits") {
                cout << "Usage: metro list commits\n";
            }
            if (args.positionals[0] == "branches") {
                cout << "Usage: metro list branches\n";
            }
        }
    }
};

