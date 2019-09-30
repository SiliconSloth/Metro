#pragma once

namespace git {

    struct ConflictIndex {
        git_index_entry ancestor;
        git_index_entry ours;
        git_index_entry theirs;
    };

    class ConflictIterator {
    public:
        explicit ConflictIterator(git_index_conflict_iterator *iterator) : iterator(iterator) {};
        ~ConflictIterator();

        bool next(ConflictIndex &out);
        void for_each(const function <void (ConflictIndex)>& f);


    private:
        git_index_conflict_iterator *iterator;
    };

}