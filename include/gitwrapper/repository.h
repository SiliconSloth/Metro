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
        [[nodiscard]] git_signature &default_signature() const;

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

        /**
         * Lookup a branch by its name in a repository.
         *
         * The branch name will be checked for validity.
         *
         * @see git_tag_create for rules about valid names.
         *
         * @param name Name of the branch to be looked-up;
         * this name is validated for consistency.
         * @param branch_type Type of the considered branch. This should
         * be valued with either GIT_BRANCH_LOCAL or GIT_BRANCH_REMOTE.
         *
         * @return The looked up branch object.
         */
        [[nodiscard]] Branch lookup_branch(const string& name, git_branch_t branchType) const;

        /**
         * Lookup a commit object from a repository.
         *
         * @param oid Identity of the commit to locate. If the object is
         *		an annotated tag it will be peeled back to the commit.
         * @return The looked up commit object.
         */
        [[nodiscard]] Commit lookup_commit(const OID& oid) const;
        [[nodiscard]] AnnotatedCommit lookup_annotated_commit(const OID& id) const;
        [[nodiscard]] OID create_commit(const string& updateRef, const git_signature &author, const git_signature &committer,
                          const string& messageEncoding, const string& message, const Tree& tree,
                          vector<Commit> parents) const;

        /**
         * Find a single object, as specified by a revision string.
         *
         * See `man gitrevisions`, or
         * http://git-scm.com/docs/git-rev-parse.html#_specifying_revisions for
         * information on the syntax accepted.
         *
         * @param spec The textual specification for an object.
         * @return Output object found
         */
        [[nodiscard]] Object revparse_single(const string& spec) const;

        /**
         * Sets the current head to the specified commit oid and optionally
         * resets the index and working tree to match.
         *
         * SOFT reset means the Head will be moved to the commit.
         *
         * MIXED reset will trigger a SOFT reset, plus the index will be replaced
         * with the content of the commit tree.
         *
         * HARD reset will trigger a MIXED reset and the working directory will be
         * replaced with the content of the index.  (Untracked and ignored files
         * will be left alone, however.)
         *
         * @param commit Commit to which the Head should be moved to. This object
         * must belong to the repo.
         * @param type Kind of reset operation to perform.
         * @param opts Checkout options to be used for a HARD reset.
         * The checkout_strategy field will be overridden (based on reset_type).
         * This parameter can be used to propagate notify and progress callbacks.
         *
         * @return 0 on success or an error code
         */
        void reset_to_commit(const Commit &, git_reset_t, git_checkout_options) const;

        /**
         * Create a new direct reference.
         *
         * A direct reference (also called an object id reference) refers directly
         * to a specific object id (a.k.a. OID or SHA) in the repository.  The id
         * permanently refers to the object (although the reference itself can be
         * moved).  For example, in libgit2 the direct ref "refs/tags/v0.17.0"
         * refers to OID 5b9fac39d8a76b9139667c26a63e6b3f204b3977.
         *
         * The direct reference will be created in the repository and written to
         * the disk.
         *
         * Valid reference names must follow one of two patterns:
         *
         * 1. Top-level names must contain only capital letters and underscores,
         *    and must begin and end with a letter. (e.g. "HEAD", "ORIG_HEAD").
         * 2. Names prefixed with "refs/" can be almost anything.  You must avoid
         *    the characters '~', '^', ':', '\\', '?', '[', and '*', and the
         *    sequences ".." and "@{" which have special meaning to revparse.
         *
         * This function will return an error if a reference already exists with the
         * given name unless `force` is true, in which case it will be overwritten.
         *
         * @param name The name of the reference.
         * @param oid The object id pointed to by the reference.
         * @param force Overwrite existing references.
         * @return Reference created as a Branch object.
         */
        [[nodiscard]] Branch create_reference(const string& name, const OID& oid, bool force) const;

        /**
         * Create a new branch pointing at a target commit
         *
         * A new direct reference will be created pointing to
         * this target commit. If `force` is true and a reference
         * already exists with the given name, it'll be replaced.
         *
         * The branch name will be checked for validity.
         * See `git_tag_create()` for rules about valid names.
         *
         * @param branch_name Name for the branch; this name is
         * validated for consistency. It should also not conflict with
         * an already existing branch name.
         * @param target Commit to which this branch should point. This object
         * must belong to the repo.
         * @param force Overwrite existing branch.
         * @return Branch object created.
         */
        [[nodiscard]] Branch create_branch(const string& branch_name, const Commit &target, bool force) const;

        /**
         * Delete an existing reference by name
         *
         * This method removes the named reference from the repository without
         * looking at its old value.
         *
         * @param name The reference to remove
         */
        void remove_reference(const string& name) const;

        /**
         * Create an iterator which loops over the requested branches.
         *
         * @param flags Filtering flags for the branch
         * listing. Valid values are GIT_BRANCH_LOCAL, GIT_BRANCH_REMOTE
         * or GIT_BRANCH_ALL.
         * @return The created iterator.
         */
        [[nodiscard]] BranchIterator new_branch_iterator(const git_branch_t& flags) const;

        /**
         * Perform a callback on each reference in the repository.
         *
         * The `callback` function will be called for each reference in the
         * repository, receiving the reference object and the `payload` value
         * passed to this method.  Returning a non-zero value from the callback
         * will terminate the iteration.
         *
         * Note that the callback function is responsible to call `git_reference_free`
         * on each reference passed to it, or place the reference into a data structure
         * with the same effect.
         *
         * @param callback Function which will be called for every listed ref.
         * @param payload Additional data to pass to the callback.
         */
        void foreach_reference(const foreach_reference_cb& callback, const void *payload) const;

        /**
         * Perform a callback on each reference in the repository whose name
         * matches the given pattern.
         *
         * This function acts like `Ropository::foreach_reference()` with an additional
         * pattern match being applied to the reference name before issuing the
         * callback function.  See that function for more information.
         *
         * The pattern is matched using fnmatch or "glob" style where a '*' matches
         * any sequence of letters, a '?' matches any letter, and square brackets
         * can be used to define character ranges (such as "[0-9]" for digits).
         *
         * @param glob Pattern to match (fnmatch-style) against reference name.
         * @param callback Function which will be called for every listed ref.
         * @param payload Additional data to pass to the callback.
         */
        void foreach_reference_glob(const string& glob, const foreach_reference_glob_cb& callback, const void *payload) const;

        /**
         * Gather file status information and populate the `StatusList`.
         *
         * Note that if a `pathspec` is given in the `git_status_options` to filter
         * the status, then the results from rename detection (if you enable it) may
         * not be accurate.  To do rename detection properly, this must be called
         * with no `pathspec` so that all files can be considered.
         *
         * @param options Status options structure.
         * @return Results of getting status list.
         */
        [[nodiscard]] StatusList new_status_list(const git_status_options& options) const;

        /**
         * Make the repository HEAD point to the specified reference.
         *
         * If the provided reference points to a Tree or a Blob, the HEAD is
         * unaltered and a GitException is thrown.
         *
         * If the provided reference points to a branch, the HEAD will point
         * to that branch, staying attached, or become attached if it isn't yet.
         * If the branch doesn't exist yet, no error will be return. The HEAD
         * will then be attached to an unborn branch.
         *
         * Otherwise, the HEAD will be detached and will directly point to
         * the Commit.
         *
         * @param name Canonical name of the reference the HEAD should point at.
         */
        void set_head(const string& name) const;

        /**
         * Make the repository HEAD directly point to the Commit.
         *
         * If the provided commitish cannot be found in the repository,
         * the HEAD is unaltered and GIT_ENOTFOUND is returned.
         *
         * If the provided commitish cannot be peeled into a commit, the HEAD is unaltered and -1 is returned.
         *
         * Otherwise, the HEAD will eventually be detached and will directly point to the peeled Commit.
         *
         * @param commitish Object id of the Commit the HEAD should point to.
         */
         void set_head_detached(const OID& commitish) const;

        /**
         * Retrieve and resolve the reference pointed at by HEAD.
         */
        Branch head() const;

        /**
         * Check if a repository's HEAD is detached.
         */
        bool head_detached() const;

        /**
         * Updates files in the index and working tree to match the content of the
         * tree pointed at by the tree.
         *
         * @param tree a commit, tag or tree which content will be used to update
         * the working directory
         * @param options Specifies checkout options.
         */
        void checkout_tree(const Tree& tree, const git_checkout_options& options) const;

        /**
         * Remove all the metadata associated with an ongoing command like merge,
         * revert, cherry-pick, etc.  For example: MERGE_HEAD, MERGE_MSG, etc.
         */
        void cleanup_state() const;

        /**
         * Analyzes the given branch(es) and determines the opportunities for
         * merging them into the HEAD of the repository.
         *
         * @param sources The sources to merge into.
         * @return An analysis of the merge.
         */
        [[nodiscard]] git_merge_analysis_t merge_analysis(const vector<AnnotatedCommit>& sources) const;

        /**
         * Merges the given commit(s) into HEAD, writing the results into the working
         * directory.  Any changes are staged for commit and any conflicts are written
         * to the index.  Callers should inspect the repository's index after this
         * completes, resolve any conflicts and prepare a commit.
         *
         * For compatibility with git, the repository is put into a merging
         * state. Once the commit is done (or if the uses wishes to abort),
         * you should clear this state by calling `Ropository::cleanup_state()`.
         *
         * @param sources The sources to merge into.
         * @param merge_opts Merge options.
         * @param checkout_opts Checkout options.
         */
        void merge(const vector<AnnotatedCommit>& sources, const git_merge_options& merge_opts, const git_checkout_options& checkout_opts) const;

        /**
         * Get a list of the configured remotes for a repo
         *
         * @return A string array which receives the names of the remotes.
         */
        [[nodiscard]] StrArray remote_list() const;

        /**
         * Add a remote with the default fetch refspec to the repository's configuration.
         *
         * @param name The remote's name.
         * @param url The remote's url.
         * @return The resulting remote.
         */
        [[nodiscard]] Remote remote_create(string name, string url) const;

        /**
         * Set the remote's url in the configuration
         *
         * Remote objects already in memory will not be affected. This assumes
         * the common case of a single-url remote and will otherwise return an error.
         *
         * @param remote The remote's name.
         * @param url The url to set.
         */
        void remote_set_url(string remote, string url) const;

        /**
         * Get the information for a particular remote
         *
         * The name will be checked for validity.
         * See `git_tag_create()` for rules about valid names.
         *
         * @param name The remote's name.
         * @return New remote object.
         */
        [[nodiscard]] Remote lookup_remote(string name) const;

        /**
         * Find a merge base between two commits
         *
         * @param one One of the commits.
         * @param two The other commit.
         * @return The OID of a merge base between 'one' and 'two'.
         */
        [[nodiscard]] OID merge_base(OID one, OID two) const;

        /**
         * Get the configuration file for this repository.
         *
         * If a configuration file has not been set, the default
         * config set for the repository will be returned, including
         * global and system configurations (if they are available).
         *
         * @return The loaded configuration.
         */
        [[nodiscard]] Config config() const;
    };

}