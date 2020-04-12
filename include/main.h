/*
 * References to variables and types used in main.cpp
 */

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