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

    string Branch::reference_name() const {
        const char *out;
        out = git_reference_name(ref.get());
        return string(out);
    }

    OID Branch::target() {
        const git_oid *oid = git_reference_target(ref.get());
        return oid == nullptr ? OID() : OID(*oid);
    }

    void Branch::set_target(OID oid, const char *log_message) {
        git_reference *out;
        git_reference_set_target(&out, ref.get(), &oid.oid, log_message);
        shared_ptr<git_reference> new_ref(out, git_reference_free);
        ref = new_ref;
    }
}