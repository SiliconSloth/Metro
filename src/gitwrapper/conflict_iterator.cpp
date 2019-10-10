namespace git {
    bool ConflictIterator::next(Conflict &out) const {
        int err = git_index_conflict_next(&out.ancestor, &out.ours, &out.theirs, iter.get());

        if(err != GIT_ITEROVER){
            check_error(err);
            return true;
        } else {
            return false;
        }
    }
}