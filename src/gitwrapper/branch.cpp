namespace git {
    string Branch::name() const {
        const char *out;
        int err = git_branch_name(&out, ref.get());
        check_error(err);
        return string(out);
    }

    string Branch::reference_name() const {
        const char *out;
        out = git_reference_name(ref.get());
        return string(out);
    }

    bool Branch::is_head() const {
        int result = git_branch_is_head(ref.get());
        check_error(result);
        return result;
    }

    OID Branch::target() const {
        const git_oid *oid = git_reference_target(ref.get());
        return oid == nullptr ? OID() : OID(*oid);
    }

    git_reference_t Branch::type() const {
        return git_reference_type(ref.get());
    }

    void Branch::set_target(OID oid, const char *log_message) {
        git_reference *out;
        git_reference_set_target(&out, ref.get(), &oid.oid, log_message);
        shared_ptr<git_reference> new_ref(out, git_reference_free);
        ref = new_ref;
    }

    void Branch::delete_branch() const {
        int err = git_branch_delete(ref.get());
        check_error(err);
    }

    void Branch::delete_reference() const {
        git_reference_delete(ref.get());
    }

    void Branch::rename(string new_branch_name, bool force) {
        int force_i = (int) force;
        git_reference *out;
        git_reference *branch = ref.get();
        int err = git_branch_move(&out, branch, new_branch_name.c_str(), force_i);
        check_error(err);
        ref.reset(out);
    }
}