#pragma once

#include "pch.h"

namespace git {

    class Index {
    private:
        shared_ptr<git_index> index;

    public:
        explicit Index(git_index *index) : index(index, git_index_free) {};

        Index() = delete;

        Index operator=(Index i) = delete;

        [[nodiscard]] shared_ptr<git_index> ptr() const {
            return index;
        }

        void add_all(StrArray pathspec, unsigned int flags, MatchedPathCallback callback);
        OID write_tree();
        void write();
        ConflictIterator conflict_iterator();
        [[nodiscard]] size_t entrycount() const;
    };

}