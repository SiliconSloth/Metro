#include "pch.h"

// Contains a list of all commands
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

// Contains a list of all valid options
const Option ALL_OPTIONS[] = {
        {"help", "h", false},
        {"timeout", "t", true},
        {"force", "f", false},
        {"soft", "s", false} 
};