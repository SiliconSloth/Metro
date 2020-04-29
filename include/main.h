/*
 * References to variables and types used in main.cpp
 */

#pragma once
#include "pch.h"

#define METRO_RELEASE "alpha-debug"
#define METRO_MAJOR 1
#define METRO_MINOR 0
#define METRO_REVISION 0

// List of all commands
Command *allCommands[] = {
        &create,
        &clone_repo,
        &commit,
        &patch,
        &deleteCmd,
        &branch,
        &switchCmd,
        &info,
        &absorbCmd,
        &resolve,
        &syncCmd,
        &listCmd,
        &sinkCmd,
        &renameCmd
};

// Defines a mistype the user may make
struct Mistype {
    string input;       // The string the user inputs
    string message;     // The message to print if that string is detected
};

// Mistypes are checked before commands.
// If the mistype is the same a command,
// the command will become impossible to run.
const Mistype ALL_MISTYPES[] = {
        {"commit -m", "Metro doesn't require the -m option for a commit message.\nDid you mean metro commit <message>?"},
        {"add", "Metro automatically adds all files in the repository to the staging area."},
        {"push", "Did you mean metro sync --push?"},
        {"pull", "Did you mean metro sync --pull?"},
        {"merge", "Did you mean metro absorb <branch>?"},
        {"checkout", "Did you mean metro switch <branch>?"},
        {"log", "Did you mean metro list commits?"},
        {"init", "Did you mean metro create?"},
        {"rm", "Metro automatically adds all files in the repository to the staging area.\nTo exclude files from a commit use git commit."},
        {"status", "Did you mean metro info?"},
        {"rebase", "Metro currently has no support for rebasing."},
        {"reset", "Metro currently has no support for resetting."}
};