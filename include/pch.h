#ifndef PCH_H
#define PCH_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <direct.h>
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
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>
#include <csignal>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#elif __unix__ || __APPLE__ || __MACH__
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <ctime>
#include <signal.h>
#endif //_WIN32
#if _WIN32 || __APPLE__ || __MACH__
#include <filesystem>
namespace std_filesystem = std::filesystem;
#elif __unix__
#include <experimental/filesystem>
namespace std_filesystem = std::experimental::filesystem;
#endif

#include "git2.h"
#if (LIBGIT2_VER_MINOR < 28)
#define git_error_last giterr_last
#endif

using std::cout;
using std::endl;
using std::flush;
using std::vector;
using std::map;
using std::string;
using std::function;

#include "commands.h"
#include "helper.h"
#include "error.h"
#include "exit.h"
#include "child_process.h"

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
#include "gitwrapper/config.h"
#include "gitwrapper/repository.h"
#include "gitwrapper/strarray.h"
#include "gitwrapper/diff.h"

#include "metro/metro.h"
#include "metro/credentials.h"
#include "metro/merging.h"
#include "metro/branch_descriptor.h"
#include "metro/syncing.h"
#include "metro/url_descriptor.h"

#endif //PCH_H
