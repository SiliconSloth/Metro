namespace git {
    string Branch::name() const {
        const char *out;
        int err = git_branch_name(&out, ref.get());
        check_error(err);
        return string(out);
    }

    bool Branch::is_head() const {
        int result = git_branch_is_head(ref.get());
        check_error(result);
        return result;
    }

    void Branch::delete_branch() {
        int err = git_branch_delete(ref.get());
        check_error(err);
    }
}