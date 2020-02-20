#include "pch.h"

Command sinkCmd{
        "sink",
        "Sinks the Metro",

        // execute
        [](const Arguments& args) {
            if (args.positionals.empty()) {
                throw MissingPositionalException("type");
            }

            cout << "Turned " << args.positionals[0] << " water" << endl;
    },

    // printHelp
    [](const Arguments& args) {
        if (args.positionals.empty() || (args.positionals[0] != "on" && args.positionals[0] != "off")) {
            cout << "Usage: metro sink <on/off>\n";
        }
        if (!args.positionals.empty()) {
            if (args.positionals[0] == "on") {
                cout << "Usage: metro sink on\n";
            }
            if (args.positionals[0] == "off") {
                cout << "Usage: metro sink off\n";
            }
        }
    }
};

