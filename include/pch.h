#ifndef PCH_H
#define PCH_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <fstream>
#include <streambuf>
#include <functional>
#include <memory>
#include <algorithm>
#include <regex>
#include <cctype>
#include <cassert>

#ifdef _WIN32
#include <windows.h>
#elif __unix__
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <iostream>
#include <fstream>
#endif //_WIN32

#include "git2.h"
#if (LIBGIT2_VER_MINOR < 28)
#define git_error_last giterr_last
#endif


#include "commands.h"
#include "helper.h"
#include "error.h"

#include "gitwrapper/types.h"
#include "gitwrapper/oid.h"
#include "gitwrapper/branch.h"
#include "gitwrapper/conflict_iterator.h"
#include "gitwrapper/index.h"
#include "gitwrapper/tree.h"
#include "gitwrapper/commit.h"
#include "gitwrapper/annotated_commit.h"
#include "gitwrapper/object.h"
#include "gitwrapper/branch_iterator.h"
#include "gitwrapper/status_list.h"
#include "gitwrapper/remote.h"
#include "gitwrapper/repository.h"
#include "gitwrapper/strarray.h"
#include "gitwrapper/diff.h"

#include "metro/metro.h"
#include "metro/credentials.h"
#include "metro/merging.h"
#include "metro/syncing.h"
#include "metro/branch_descriptor.h"

#endif //PCH_H
