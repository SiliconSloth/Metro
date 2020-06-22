namespace git {
    Treebuilder Treebuilder::create(const Repository &repo) {
        git_treebuilder *tb;
        int err = git_treebuilder_new(&tb, repo.ptr().get(), nullptr);
        check_error(err);
        return Treebuilder(tb);
    }

    Treebuilder Treebuilder::create(const Repository &repo, Tree tree) {
        git_treebuilder *tb;
        int err = git_treebuilder_new(&tb, repo.ptr().get(), tree.ptr().get());
        check_error(err);
        return Treebuilder(tb);
    }

    OID Treebuilder::write() {
        git_oid oid;
        int err = git_treebuilder_write(&oid, tb.get());
        check_error(err);
        return OID(oid);
    }
}