namespace git {
    ConflictIterator::~ConflictIterator() {
        git_index_conflict_iterator_free(iterator);
    }

    bool ConflictIterator::next(ConflictIndex &out)  {
        int err = git_index_conflict_next(reinterpret_cast<const git_index_entry **>(&out.ancestor),
                                reinterpret_cast<const git_index_entry **>(&out.ours),
                                reinterpret_cast<const git_index_entry **>(&out.theirs), iterator);
        if(err != GIT_ITEROVER){
            check_error(err);
            return true;
        } else {
           return false;
        }
    }

    void ConflictIterator::for_each(const function <void (ConflictIndex)>& f) {
        ConflictIndex index {};
        for (; next(index);) {
            f(index);
        }
    }
}