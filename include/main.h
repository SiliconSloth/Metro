#include "pch.h"

Command *allCommands[] = {
        &create,
        &commit,
        &patch
};

const Option ALL_OPTIONS[] = {
        {"help", "h", false},
        {"timeout", "t", true},
        {"force", "f", false}
};