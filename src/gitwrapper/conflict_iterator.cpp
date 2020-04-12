    namespace git {
    bool ConflictIterator::next(Conflict &out) const {
        const git_index_entry *ancestor;
        const git_index_entry *ours;
        const git_index_entry *theirs;

        int err = git_index_conflict_next(&ancestor, &ours, &theirs, iter.get());
        if(err == GIT_ITEROVER) {
            return false;
        }
        check_error(err);

        out = Conflict(ancestor, ours, theirs);
        return true;
    }
}