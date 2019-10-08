#include "pch.h"

namespace metro {
    string get_merge_message(const Repository& repo) {
        return read_all(repo.path() + "/MERGE_MSG");
    }
}