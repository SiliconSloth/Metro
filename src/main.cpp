/*
 * The start of the program and files relating to parsing the input and allocating tasks to the individual commands.
 */

#include "pch.cpp"
#include "main.h"

using namespace std;

/**
 * Retrieve the Option corresponding to a flag passed by the user.
 * @param flag Flag passed to metro via commandline.
 * @return Option corresponding with flag passed.
 * @throws UnknownOptionException if the option doesn't exist.
 *
 * If the flag starts with "--" the full option name is expected,
 * if it starts with only "-" the contraction is expected.
 * Raises an exception if no matching Option is found.
 */
Option lookup_option(string const& flag) {
    bool usedContraction = !has_prefix(flag, "--");
    // Remove -- or -
    string name = flag.substr(usedContraction ? 1:2, string::npos);
    for (const auto & option : ALL_OPTIONS) {
        if ((option.contraction == name && usedContraction)
         || (option.name == name && !usedContraction)) {
            return option;
        }
    }
    throw UnknownOptionException(flag);
}

/**
 * Parse the arguments given to Metro on the command line.
 * @param argc Number of arguments.
 * @param argv The arguments to parse.
 * @return Formal arguments format.
 * @throws MissingValueException If the option requires a value but does not have one.
 * @throws UnexpectedValueException If the option doesn't require a value but one was found.
 * @throws MissingFlagException A value was found without a flag attached to it
 *
 * All positional arguments must come before all Option arguments.
 * Options may have a value associated with them, in the form "--key=value" or "--key value".
 * Each option has a long version, prefixed with --, and a short version, prefixed with -.
 * Using the wrong prefix will result in the Option not being recognised.
 * The --help and -h flags are excluded from the options; instead hashHelpFlag is set.
 * The mame of the executable is excluded from the returned arguments.
 *
 * The returned option map maps the long name of each option to its value,
 * even if a contraction is used. Prefix -'s are excluded.
 *
 * <ul>
 * Will throw an exception if:
 *     <li>An unknown option is given
 *     <li>An option that requires a value isn't given one</li>
 *     <li>An option that doesn't require a value is given one</li>
 *     <li>A positional argument is found after an option (interpreted as a value with no corresponding option flag)</li>
 * <ul>
 */
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
                throw MissingValueException(string(argv[i - 1]));
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
                throw MissingFlagException(arg);
            }
        }
    }

    // Make sure the last option had a value.
    if (optionOpen) {
        throw MissingValueException(string(argv[argc - 1]));
    }

    return args;
}

/**
 * Prints a generic help message with all commands listed
 */
void printHelp() {
    cout << "Usage: metro <command> <args> [options]\n";
    for (const Command *cmd : allCommands) {
        if (cmd->name == "sink") continue; // Don't list sink in help message
        cout << cmd->name << " - " << cmd->description << "\n";
    }
    cout << "Use --help for help.\n";
}

/**
 * Checks the input for any matches in ALL_MISTYPES (main.h)
 * at the start of the arguments. If a match is found, the message
 * will be printed and return true.
 *
 * @param argc Number of arguments.
 * @param argv The arguments to parse.
 * @return True if a match was found.
 */
bool mistype_check(int argc, char *argv[]) {
    stringstream s;
    for (int i = 1; i < argc; i++) {
        s << argv[i] << " ";
    }
    string input = s.str();
    for (Mistype m : ALL_MISTYPES) {
        if (input.find(m.input) == 0) {
            cout << m.message << endl;
            return true;
        }
    }
    return false;
}

/**
 * Entry point of the program, passing off parsing to above functions
 */
int main(int argc, char *argv[]) {
    // Setup signal handling to cancel syncing on Ctrl+C pressed
#ifdef _WIN32
    SetConsoleCtrlHandler(on_application_exit, TRUE); // Ignore failure to set exit activity
#elif __unix__ || __APPLE__ || __MACH__
    signal(SIGINT, on_application_exit);
#endif //_WIN32

    git_libgit2_init();

    // Windows terminals don't all work out the box
#ifdef _WIN32
    // If terminal is xterm, we know colours work normally
    t_ops.term = get_env("TERM");
    if (t_ops.term.empty()) {
        // Disable ANSI if it would error normally.
        try {
            enable_ansi();
            disable_ansi();

            // If success, assume correct Windows terminal with all features
        } catch (ANSIException &e) {
            t_ops.ansi_enabled = false;
            t_ops.progress_enabled = false;
            t_ops.ansi_colour_change = false;
        }
    } else if (t_ops.term == "xterm") {
        // xterm is known to work with ANSI colour-codes
        t_ops.ansi_enabled = false;
        t_ops.progress_enabled = false;
        t_ops.ansi_colour_change = true;
    } else {
        // If term is unknown, assume the worst
        t_ops.ansi_enabled = false;
        t_ops.progress_enabled = false;
        t_ops.ansi_colour_change = false;
    }
#endif //_WIN32

    if (mistype_check(argc, argv)) return -1;

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
                    // Handle exceptions that may come up with labels
                    try {
                        cmd->execute(args);
                        return 0;
                    } catch (CommandArgumentException& e) {
                        cout << e.what() << "\n";
                        cmd->printHelp(args);
                        return -1;
                    } catch (MetroException& e) {
                        cout << e.what() << "\n";
                        return -1;
                    } catch (GitException& e) {
                        // If Ctrl~C was issued, Git errpr is expected and should be ignored
                        if (exit_config.received) return 0;
                        cout << "Git Error: " << e.what() << "\n";
                        return -1;
                    } catch (exception& e) {
                        cout << "Internal Error: " << e.what() << "\n";
                        return -1;
                    }
                }
            }
        }
        // Command not found
        cout << "Invalid command: " << argCmd << "\n";
        printHelp();
        return -1;
    } catch (const CommandArgumentException& e) {
        cout << e.what() << "\n";
        printHelp();
        return -1;
    }
}
