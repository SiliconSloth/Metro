namespace git {
    void Remote::fetch(const StrArray& refspecs, git_fetch_options opts) {
        int err = git_remote_fetch(remote.get(), refspecs.ptr().get(), &opts, nullptr);
        check_error(err);
    }

    void Remote::push(StrArray refspecs, git_push_options opts) const {
        int err = git_remote_push(remote.get(), refspecs.ptr().get(), &opts);
        check_error(err);
    }
}