namespace git {

    Index::~Index() {
        git_index_free(index);
    }

    void Index::add_all(const StrArray pathspec, unsigned int flags, MatchedPathCallback callback) {
        int err = git_index_add_all(index, &pathspec, flags, callback, nullptr);
        check_error(err);
    }

    OID Index::write_tree() {
        git_oid oid;
        int err = git_index_write_tree(&oid, index);
        check_error(err);
        return oid;
    }

    void Index::write() {
        git_index_write(index);
    }

    ConflictIterator Index::conflict_iterator() {
        git_index_conflict_iterator *it;
        int err = git_index_conflict_iterator_new(&it, index);
        return ConflictIterator(it);
    }

    int Index::entrycount() const {
        return git_index_entrycount(index.get())
    }

}