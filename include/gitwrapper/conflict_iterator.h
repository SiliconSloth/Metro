#pragma once

namespace git {

    struct Conflict {
        const git_index_entry *ancestor;
        const git_index_entry *ours;
        const git_index_entry *theirs;
    };

    class ConflictIterator {
    private:
        shared_ptr<git_index_conflict_iterator> iter;
        Branch lastItem;
        bool cached = false;

    public:
        explicit ConflictIterator(git_index_conflict_iterator *iter) : iter(iter, git_index_conflict_iterator_free) {}

        ConflictIterator() = delete;

        [[nodiscard]] shared_ptr<git_index_conflict_iterator> ptr() {
            return iter;
        }

        bool next(Conflict& out) const;
    };

}