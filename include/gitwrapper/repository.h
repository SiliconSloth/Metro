/*
 * Contains wrapper for git_repository type.
 */

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

    /**
     * Representation of an existing git repository, including all its object contents.
     */
    class Repository {
    private:
        explicit Repository(git_repository *repo) : repo(repo, git_repository_free) {}

        shared_ptr<git_repository> repo;

    public:
        Repository() = delete;

        [[nodiscard]] shared_ptr<git_repository> ptr() const {
            return repo;
        }

        /**
         * Creates a new Git repository in the given folder.
         *
         * @param path Folder to create repo in.
         * @param isBare if true, a Git repository without a working directory is
         * created at the pointed path. If false, provided path will be
         * considered as the working directory into which the .git directory
         * will be created.
         * @return The created repository.
         */
        static Repository init(const string& path, bool isBare);

        /**
         * Open a git repository.
         *
         * The 'path' argument must point to either a git repository
         * folder, or an existing work dir.
         *
         * The method will automatically detect if 'path' is a normal
         * or bare repository or fail is 'path' is neither.

         * @param path The path to the repository.
         * @return The opened repository object.
         */
        static Repository open(const string& path);

        /**
         * Clone a remote repository.
         *
         * By default this creates its repository and initial remote to match
         * git's defaults. You can use the options in the callback to
         * customize how these are created.
         *
         * @param url The remote repository to clone.
         * @param path Local directory to clone to.
         * @param options Configuration options for the clone.  If NULL, the
         *        function works as though GIT_OPTIONS_INIT were passed.
         * @return Cloned repo.
         */
        static Repository clone(const string& url, const string& path, git_clone_options *options);

        /**
         * Checks if a repo exists as the given path.
         * @param path Path to check.
         * @return True if repo exists at path.
         */
        static bool exists(const string& path);

        /**
         * Gets the path of the repository.
         *
         * @return Repo path, ending with `/.git`
         */
        [[nodiscard]] string path() const;

        /**
         * Create a new action signature with default user and now timestamp.
         *
         * This looks up the user.name and user.email from the configuration and
         * uses the current time as the timestamp, and creates a new signature
         * based on that information.
         *
         * @return New signature.
         */
        [[nodiscard]] Signature &default_signature() const;

        /**
         * Get the Index file for this repository.
         *
         * If a custom index has not been set, the default
         * index for the repository will be returned (the one
         * located in `.git/index`).
         *
         * @return Loaded index object.
         */
        [[nodiscard]] Index index() const;

        /**
         * Lookup a tree object from the repository.
         *
         * @param oid Identity of the tree to locate.
         * @return Looked up tree object.
         */
        [[nodiscard]] Tree lookup_tree(const OID &oid) const;

        /**
         * Lookup a reference by name in a repository.
         *
         * The name will be checked for validity.
         * See `git_reference_symbolic_create()` for rules about valid names.
         *
         * @param name The long name for the reference (e.g. HEAD, refs/heads/master, refs/tags/v0.1.0, ...).
         * @return Looked up reference.
         */
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