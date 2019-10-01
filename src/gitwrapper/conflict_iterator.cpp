namespace git {
    ConflictIterator::~ConflictIterator() {
        git_index_conflict_iterator_free(iterator);
    }

    bool ConflictIterator::has_next() {
        bool out = next(temp);
        if(out) {
            cached = true;
        }
        return out;
    }

    bool ConflictIterator::next(ConflictIndex &out)  {
        if(cached) {
            out.ancestor = temp.ancestor;
            out.ours = temp.ours;
            out.theirs = temp.theirs;
            cached = false;
            return true;
        }

        int err = git_index_conflict_next(&out.ancestor, &out.ours, &out.theirs, iterator);
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