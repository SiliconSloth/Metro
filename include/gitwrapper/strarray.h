#pragma once

#include <git2/strarray.h>
#include <memory>

#define STR_EMPTY StrArray()

class StrArray {
public:
    git_strarray array{};
    StrArray() {
        array.count = 0;
        array.strings = nullptr;
    }
    explicit StrArray(git_strarray);
    void free();
    char **strings();
    size_t count();
};
