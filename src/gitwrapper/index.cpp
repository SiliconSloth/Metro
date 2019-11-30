namespace git {

    void Index::add_all(const StrArray& pathspec, unsigned int flags, MatchedPathCallback callback) {
        git_strarray ps = *pathspec.ptr();
        int err = git_index_add_all(index.get(), &ps, flags, callback, nullptr);
        check_error(err);
    }

    OID Index::write_tree() {
        git_oid oid;
        int err = git_index_write_tree(&oid, index.get());
        check_error(err);
        return OID(oid);
    }

    void Index::write() {
        git_index_write(index.get());
    }

    ConflictIterator Index::conflict_iterator() const {
        git_index_conflict_iterator *it;
        int err = git_index_conflict_iterator_new(&it, index.get());
        check_error(err);
        return ConflictIterator(it);
    }

    size_t Index::entrycount() const {
        return git_index_entrycount(index.get());
    }

    void Index::add_conflict(const git::Conflict &conflict) const {
        int err = git_index_conflict_add(index.get(), conflict.ancestor, conflict.ours, conflict.theirs);
        check_error(err);
    }

    void Index::cleanup_conflicts() const {
        int err = git_index_conflict_cleanup(index.get());
        check_error(err);
    }

    bool Index::has_conflicts() const {
        return git_index_has_conflicts(index.get());
    }
}