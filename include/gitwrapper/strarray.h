#pragma once

#include <git2/strarray.h>
#include <memory>

namespace git {
    void free_strarray(git_strarray *array) {
        git_strarray_free(array);
        delete array;
    }

    class StrArray {
    private:
        const shared_ptr<git_strarray> array;

    public:
        explicit StrArray(git_strarray *array) : array(new git_strarray{array->strings, array->count}, git::free_strarray) {}

        StrArray() : array(new git_strarray{nullptr, 0}) {}

        StrArray operator=(StrArray c) = delete;

        [[nodiscard]] char** strings() const {
            return array.get()->strings;
        }

        [[nodiscard]] size_t count() const {
            return array.get()->count;
        }
    };
}
