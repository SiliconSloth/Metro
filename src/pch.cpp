#include "pch.h"

#include "helper.cpp"
#include "error.cpp"
#include "child_process.cpp"

#include "gitwrapper/oid.cpp"
#include "gitwrapper/index.cpp"
#include "gitwrapper/branch.cpp"
#include "gitwrapper/branch_iterator.cpp"
#include "gitwrapper/repository.cpp"
#include "gitwrapper/remote.cpp"
#include "gitwrapper/commit.cpp"
#include "gitwrapper/conflict_iterator.cpp"
#include "gitwrapper/diff.cpp"
#include "gitwrapper/config.cpp"

#include "metro/metro.cpp"
#include "metro/merging.cpp"
#include "metro/credentials.cpp"
#include "metro/syncing.cpp"
#include "metro/branch_descriptor.cpp"
#include "metro/url_descriptor.cpp"

#include "commands/create.cpp"
#include "commands/clone.cpp"
#include "commands/commit.cpp"
#include "commands/patch.cpp"
#include "commands/delete.cpp"
#include "commands/branch.cpp"
#include "commands/switch.cpp"
#include "commands/info.cpp"
#include "commands/absorb.cpp"
#include "commands/resolve.cpp"
#include "commands/sync.cpp"
#include "commands/list.cpp"