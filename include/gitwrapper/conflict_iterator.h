#pragma once

namespace git {

    struct ConflictIndex {
        const git_index_entry *ancestor;
        const git_index_entry *ours;
        const git_index_entry *theirs;
    };

    class ConflictIterator {
    public:
        explicit ConflictIterator(git_index_conflict_iterator *iterator) : iterator(iterator) {};
        ~ConflictIterator();

        bool next(ConflictIndex &out);
        bool has_next();
        void for_each(const function <void (ConflictIndex)>& f);

    private:
        git_index_conflict_iterator *iterator;
        ConflictIndex temp;
        bool cached = false;
    };

}