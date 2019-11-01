#include "pch.h"

Command *allCommands[] = {
        &create,
        &commit,
        &patch,
        &deleteCmd,
        &branch,
        &switchCmd,
        &info,
        &absorbCmd,
        &resolve,
        &syncCmd
};

const Option ALL_OPTIONS[] = {
        {"help", "h", false},
        {"timeout", "t", true},
        {"force", "f", false}
};