#ifndef PCH_H
#define PCH_H

using namespace std;

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

#include "git2.h"

#include "commands.h"
#include "helper.h"
#include "error.h"

#include "gitwrapper/types.h"
#include "gitwrapper/index.h"
#include "gitwrapper/commit.h"
#include "gitwrapper/object.h"
#include "gitwrapper/branch.h"
#include "gitwrapper/branch_iterator.h"
#include "gitwrapper/status_list.h"
#include "gitwrapper/repository.h"
#include "gitwrapper/conflict_iterator.h"

#include "metro/metro.h"
#include "metro/merging.h"

#endif //PCH_H
