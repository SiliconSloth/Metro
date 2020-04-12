/*
 * Defines the List command.
 */

/**
 * Prints the details of a given commit to the console.
 *
 * @param nth_parent The commit to print out.
 * @param hConsole The console to print using on Windows.
 */
void print_details(git::Commit nth_parent, void *hConsole) {
    set_text_colour("rg------f", hConsole);
    cout << "Commit " << nth_parent.id().str() << endl;
    set_text_colour("rgb-----r", hConsole);

    git_signature author = nth_parent.author();
    cout << "Author: " << author.name << " (" << author.email << ")" << endl;
    cout << "Date: " << time_to_string(author.when) << endl;
    cout << "\n    " << nth_parent.message() << endl;
}

/**
 * Prints the given commit followed by all parent commits with a prompt before each commit.
 *
 * @param commit Commit to begin printing from (Inclusive).
 * @param hConsole The console to print using on Windows.
 */
void print_from_commit(git::Commit commit, void *hConsole) {
    unsigned int count = commit.parentcount();
    print_details(commit, hConsole);
    cout << endl;
    for (unsigned int i = 0; i < count; i++) {
        git::Commit nth_parent = commit.parent(i);

        while (true) {
            cout << ":" << flush;
            char next = getchar();
            enable_ansi();
            cout << "\033[1A";
            clear_line();
            disable_ansi();
            if (next == '\n') { print_from_commit(nth_parent, hConsole); break; }
            else if (next == 'q') return;
        }

        if (i != count - 1) cout << endl;
    }
}

/**
 * The list command is used to print a list of commmits or branches.
 */
Command listCmd{
        "list",
        "Lists the commits or branches",

        // execute
        [](const Arguments& args) {
            if (args.positionals.empty()) {
                throw MissingPositionalException("type");
            }

            git::Repository repo = git::Repository::open(".");
            metro::assert_not_merging(repo);

            void* hConsole;
#ifdef _WIN32
            hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif // _WIN32

            if (args.positionals[0] == "commits") {
                if (args.positionals.size() > 1) {
                    throw UnexpectedPositionalException(args.positionals[1]);
                }

                git::Commit commit = metro::get_commit(repo, "HEAD");
                print_from_commit(commit, hConsole);
            } else if (args.positionals[0] == "branches") {
                if (args.positionals.size() > 1) {
                    throw UnexpectedPositionalException(args.positionals[1]);
                }

                git::BranchIterator iter = repo.new_branch_iterator(GIT_BRANCH_LOCAL);
                std::string current = metro::current_branch_name(repo);
                for (git::Branch branch; iter.next(&branch);) {
                    std::string name = branch.name();
                    if (current == name){
                        cout << " * ";
                        set_text_colour("-g------f", hConsole);
                        cout << branch.name() << endl;
                        set_text_colour("rgb-----r", hConsole);
                    } else if (metro::is_wip(name)) {
                        continue;
                    } else if (name.find('#') != std::string::npos) {
                        cout << "   " << name.substr(0, name.find('#'));
                        set_text_colour("-gb-----f", hConsole);
                        cout << "#" << name.substr(name.find('#') + 1) << endl;
                        set_text_colour("rgb-----r", hConsole);
                    } else {
                        bool isWip = false;
                        git::BranchIterator iter2 = repo.new_branch_iterator(GIT_BRANCH_LOCAL);
                        for (git::Branch branch2; iter2.next(&branch2);) {
                            if (branch2.name() == metro::to_wip(name)) {
                                isWip = true;
                                break;
                            }
                        }
                        cout << "   " << name;
                        if (isWip) {
                            set_text_colour("--bi----f", hConsole);
                            cout << " (WIP)";
                            set_text_colour("rgb-----r", hConsole);
                        }
                        cout << endl;
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

