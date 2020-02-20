#include "pch.h"

void print_details(Commit nth_parent, void *hConsole) {
    set_text_colour("rg------f", hConsole);
    cout << "Commit " << nth_parent.id().str() << endl;
    set_text_colour("rgb-----r", hConsole);

    Signature author = nth_parent.author();
    cout << "Author: " << author.name << " (" << author.email << ")" << endl;
    cout << "Date: " << time_to_string(author.when) << endl;
    cout << "\n    " << nth_parent.message() << endl;
}

void print_from_commit(Commit commit, void *hConsole) {
    unsigned int count = commit.parentcount();
    print_details(commit, hConsole);
    cout << endl;
    for (unsigned int i = 0; i < count; i++) {
        Commit nth_parent = commit.parent(i);

        print_from_commit(nth_parent, hConsole);

        if (i != count - 1) cout << endl;
    }
}

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

            void* hConsole;
#ifdef _WIN32
            hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif // _WIN32

            if (args.positionals[0] == "commits") {
                if (args.positionals.size() > 1) {
                    throw UnexpectedPositionalException(args.positionals[1]);
                }

                Commit commit = metro::get_commit(repo, "HEAD");
                print_from_commit(commit, hConsole);
            } else if (args.positionals[0] == "branches") {
                if (args.positionals.size() > 1) {
                    throw UnexpectedPositionalException(args.positionals[1]);
                }

                BranchIterator iter = repo.new_branch_iterator(GIT_BRANCH_LOCAL);
                std::string current = metro::current_branch_name(repo);
                for (Branch branch; iter.next(&branch);) {
                    std::string name = branch.name();
                    if (current == name){
                        cout << " * ";
                        set_text_colour("-g------f", hConsole);
                        cout << branch.name() << endl;
                        set_text_colour("rgb-----r", hConsole);
                    } else if (metro::is_wip(name) && metro::to_wip(current) == name) {
                        cout << " ~ ";
                        set_text_colour("r-------f", hConsole);
                        cout << current << endl;
                        set_text_colour("rg------f", hConsole);
                        cout << " (WIP BRANCH)" << endl;
                        set_text_colour("rgb-----r", hConsole);
                    } else if (metro::is_wip(name)) {
                        cout << " ~ ";
                        set_text_colour("--bi----f", hConsole);
                        cout << metro::un_wip(name) << " (WIP BRANCH)" << endl;
                        set_text_colour("rgb-----r", hConsole);
                    } else if (has_prefix(name, current + "#")) {
                        cout << " + ";
                        set_text_colour("-gbi----f", hConsole);
                        cout << current << " <- " << name.substr(name.find("#") + 1) << endl;
                        set_text_colour("rgb-----r", hConsole);
                    } else {
                        cout << " - " << name << endl;
                    }
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

