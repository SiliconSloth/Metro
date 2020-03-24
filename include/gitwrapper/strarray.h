#pragma once

#include <git2/strarray.h>
#include <memory>
#ifdef __unix__
#define strcpy_s(dest, size, src) strcpy(dest, src)
#endif //__unix__

namespace git {
    class StrArray {
    private:
        const shared_ptr<git_strarray> array;

        static void free_strarray(git_strarray *array) {
            git_strarray_free(array);
            delete array;
        }

        static git_strarray* list_to_strarray(const vector<string>& list) {
            char **strings = new char*[list.size()];
            for (int i = 0; i < list.size(); i++) {
                strings[i] = new char[list[i].size() + 1];
                strncpy(strings[i], list[i].c_str(), list[i].size() + 1);
            }
            return new git_strarray{strings, list.size()};
        }

    public:
        explicit StrArray(git_strarray *array) : array(new git_strarray{array->strings, array->count}, free_strarray) {}
        explicit StrArray(const vector<string>& elements) : array(list_to_strarray(elements), free_strarray) {}

        StrArray() : array(new git_strarray{nullptr, 0}) {}

        StrArray operator=(StrArray c) = delete;

        [[nodiscard]] shared_ptr<git_strarray> ptr() const {
            return array;
        }

        [[nodiscard]] char** strings() const {
            return array->strings;
        }

        [[nodiscard]] size_t count() const {
            return array->count;
        }
    };
}
