/*
 * References to variables and types used in main.cpp
 */

#pragma once
#include "pch.h"

#define METRO_STAGE "alpha"
#define METRO_MAJOR 1
#define METRO_MINOR 2
#define METRO_DEVELOPMENT_BUILD

// List of all commands
Command *allCommands[] = {
        &clone_repo,
        &deleteCmd,
        &renameCmd,
        &sinkCmd,
        &switchCmd,
        &syncCmd,
        &wip
};
