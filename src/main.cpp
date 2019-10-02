#include "pch.cpp"
#include "main.h"

using namespace std;

// Retrieve the Option corresponding to a flag passed by the user.
// If the flag starts with "--" the full option name is expected,
// if it starts with only "-" the contraction is expected.
// Raises an exception if no matching Option is found.
Option lookup_option(string const& flag) {
    bool usedContraction = !has_prefix(flag, "--");
    // Remove -- or -
    string name = flag.substr(usedContraction? 1:2, string::npos);
    if (usedContraction) {
        for (unsigned long long i = 0; i < sizeof(ALL_OPTIONS); i++)
            if (ALL_OPTIONS[i].contraction == name)
                return ALL_OPTIONS[i];
    } else {
        for (unsigned long long i = 0; i < sizeof(ALL_OPTIONS); i++)
            if (ALL_OPTIONS[i].name == name)
                return ALL_OPTIONS[i];
    }
    throw UnknownOptionException(flag);
}

// Parse the arguments given to Metro on the command line.
// argc: Number of arguments.
// argv: The arguments to parse.
//
// All positional arguments must come before all Option arguments.
// Options may have a value associated with them, in the form "--key=value" or "--key value".
// Each option has a long version, prefixed with --, and a short version, prefixed with -.
// Using the wrong prefix will result in the Option not being recognised.
// The --help and -h flags are excluded from the options; instead hashHelpFlag is set.
// The mame of the executable is excluded from the returned arguments.
//
// The returned option map maps the long name of each option to its value,
// even if a contraction is used. Prefix -'s are excluded.
//
// Will throw an exception if:
// - An unknown option is given
// - An option that requires a value isn't given one
// - An option that doesn't require a value is given one
// - A positional argument is found after an option (interpreted as a value with no corresponding option flag)
Arguments parse_args(int argc, char *argv[]) {
    Arguments args {};

    // The last parsed option flag.
    Option option;
    // True if the last option parsed had no value but needed one.
    // The next argument is assigned as its value.
    bool optionOpen = false;
    // Whether we have yet to reach the start of the option arguments.
    bool acceptingPositionals = true;
    // Start at 1 to exclude the executable name.
    for (int i = 1; i < argc; i++) {
        // If this is an option flag... (long or contracted, they both start with -)
        string arg(argv[i]);
        if (has_prefix(arg, "-")) {
            // Once an option is found, stop allowing positionals.
            acceptingPositionals = false;
            // If the last option still needs a value, the argument directly after it can't also be an option name.
            if (optionOpen) {
                throw NoValueException(string(argv[i-1]));
            }

            // If this option had a value specified with --key=value, extract it.
            // If not the value will be "" and should be assigned by the next argument.
            string key;
            string value;
            split_at_first(arg, '=', key, value);

            option = lookup_option(key);
            if (!value.empty()) {
                if (option.needsValue) {
                    args.options[option.name] = value;
                } else {
                    throw UnexpectedValueException(key);
                }
            } else if (option.needsValue) {
                // If no value was given but this option needs one, consume the next argument as the value.
                optionOpen = true;
            } else {
                if (option.name == "help") {
                    // Rather than including the --help flag in the options, set hasHelpFlag.
                    args.hasHelpFlag = true;
                } else {
                    // Option is present but has no value.
                    args.options[option.name] = "";
                }
            }
        } else if (optionOpen) {
            //  If the last option had no value provided with = but needed one, this argument becomes its value.
            args.options[option.name] = arg;
            optionOpen = false;
        } else {
            if (acceptingPositionals) {
                args.positionals.push_back(arg);
            } else {
                // If a positional argument is found after the options have started,
                // we assume it is a value missing an option key.
                throw NoFlagException(arg);
            }
        }
    }

    // Make sure the last option had a value.
    if (optionOpen)
        throw NoValueException(string(argv[argc-1]));

    return args;
}

// Prints a generic help command with all commands listed
void printHelp() {
    cout << "Usage: metro <command> <args> [options]\n";
    for (const Command *cmd : allCommands) {
        cout << cmd->name << " - " << cmd->description << "\n";
    }
    cout << "Use --help for help.\n";
}

int main(int argc, char *argv[]) {
    git_libgit2_init();

    try {
        Arguments args = parse_args(argc, argv);
        // If there is no command specified, just print help and quit.
        if (args.positionals.empty()) {
            printHelp();
            return 0;
        }

        string argCmd = args.positionals[0];
        // Remove the sub-command, so we only have the arguments to the sub-command.
        args.positionals.erase(args.positionals.begin());
        for (const Command *cmd : allCommands) {
            if (cmd->name == argCmd) {
                if (args.hasHelpFlag) {
                    cmd->printHelp(args);
                    return 0;
                } else {
                    try {
                        cmd->execute(args);
                        return 0;
                    } catch (MetroException& e) {
                        cout << e.what() << "\n";
                        cmd->printHelp(args);
                        return -1;
                    } catch (GitException& e) {
                        cout << "Git Error: " << e.what() << "\n";
                        cmd->printHelp(args);
                        return -1;
                    } catch (exception& e) {
                        cout << "Internal Error: " << e.what() << "\n";
                        cmd->printHelp(args);
                        return -1;
                    }
                }
            }
        }
        cout << "Invalid command: " << argCmd << "\n";
        printHelp();
        return -1;
    } catch (const exception& e) {
        cout << e.what() << "\n";
        printHelp();
        return -1;
    }
}
