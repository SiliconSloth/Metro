#include "pch.h"

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
        &sinkCmd
};

// List of all valid options
// Keep them in alphabetical order to make help messages easier to read
const Option ALL_OPTIONS[] = {
        {"force", "f", false, "Not yet implemented"},
        {"help", "h", false, "Explain how to use command"},
        {"pull", "d", false, "Only pull changes, without pushing changes to remote"},
        {"push", "u", false, "Only push changes, without pulling changes from remote. Requires no conflicts"},
        {"soft", "s", false, "Delete the last commit without reverting changes in the working directory"}
};