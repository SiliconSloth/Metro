#pragma once

#include "pch.h"
#include "strarray.h"

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

        void add_all(const StrArray& pathspec, unsigned int flags, MatchedPathCallback callback);
        OID write_tree();
        void write();

        [[nodiscard]] ConflictIterator conflict_iterator() const;
        [[nodiscard]] size_t entrycount() const;

        void add_conflict(const Conflict& conflict) const;
        void cleanup_conflicts() const;
        [[nodiscard]] bool has_conflicts() const;
    };

}