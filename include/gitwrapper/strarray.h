#pragma once

#include <git2/strarray.h>

#define STR_EMPTY StrArray()

class StrArray {
public:
    git_strarray array{};
    StrArray() {
        array.count = 0;
        array.strings = nullptr;
    }
    StrArray(git_strarray);
    ~StrArray();
    char **strings();
    size_t count();
};
