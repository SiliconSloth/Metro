#pragma once

namespace git {
    typedef int (*foreach_reference_cb)(const Branch& reference, const void *payload);

    struct foreach_reference_cb_payload {
        const foreach_reference_cb callback;
        const void *payload;
    };

    typedef int (*foreach_reference_glob_cb)(const string& name, const void *payload);

    struct foreach_reference_glob_cb_payload {
        const foreach_reference_glob_cb callback;
        const void *payload;
    };

    class Repository {
    private:
        explicit Repository(git_repository *repo) : repo(repo, git_repository_free) {}

        shared_ptr<git_repository> repo;

    public:
        Repository() = delete;

        [[nodiscard]] shared_ptr<git_repository> ptr() const {
            return repo;
        }

        static Repository init(const string& path, bool isBare);
        static Repository open(const string& path);
        static Repository clone(const string& url, const string& path, git_clone_options *options);
        static bool exists(const string& path);

        [[nodiscard]] string path() const;
        [[nodiscard]] Signature &default_signature() const;
        [[nodiscard]] Index index() const;

        [[nodiscard]] Tree lookup_tree(const OID &oid) const;
        [[nodiscard]] Branch lookup_reference(const string& name) const;
        [[nodiscard]] Branch lookup_branch(const string& name, git_branch_t branchType) const;
        [[nodiscard]] Commit lookup_commit(const OID& oid) const;
        [[nodiscard]] AnnotatedCommit lookup_annotated_commit(const OID& id) const;
        OID create_commit(const string& updateRef, const Signature &author, const Signature &committer,
                          const string& messageEncoding, const string& message, const Tree& tree,
                          vector<Commit> parents) const;
        Object revparse_single(const string& spec) const;

        void reset_to_commit(const Commit &, ResetType, CheckoutOptions) const;

        Branch create_reference(const string& name, const OID& oid, const bool force) const;

        void create_branch(const string& branch_name, const Commit &target, bool force) const;

        void remove_reference(const string& name) const;

        [[nodiscard]] BranchIterator new_branch_iterator(const git_branch_t& flags) const;

        void foreach_reference(const foreach_reference_cb& callback, const void *payload) const;
        void foreach_reference_glob(const string& glob, const foreach_reference_glob_cb& callback, const void *payload) const;

        [[nodiscard]] StatusList new_status_list(const git_status_options& options) const;

        void set_head(const string& name) const;

        void checkout_tree(const Tree& tree, const git_checkout_options& options) const;

        void cleanup_state() const;

        [[nodiscard]] git_merge_analysis_t merge_analysis(const vector<AnnotatedCommit>& sources) const;
        void merge(const vector<AnnotatedCommit>& sources, const git_merge_options& merge_opts, const git_checkout_options& checkout_opts) const;

        StrArray remote_list() const;

        Remote remote_create(string name, string url) const;

        void remote_set_url(string remote, string url) const;

        Remote lookup_remote(string name) const;

        OID merge_base(OID one, OID two) const;

        Config config() const;
    };

}