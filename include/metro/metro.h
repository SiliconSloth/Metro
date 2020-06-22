/*
 * General Metro-specific functionality relates to commands, but no specific branch of functionality.
 */

#pragma once

namespace metro {
    using namespace git;

    /**
     * Tests if the repo is currently merging.
     *
     * @param repo The repository to test.
     * @return True if the repo is currently merging.
     */
    bool merge_ongoing(const Repository& repo);

    /**
     * Asserts that there is not an ongoing merge.
     *
     * @param repo The repo to test for merge on.
     * @throws CurrentlyMergingException if a merge is currently taking place.
     */
    void assert_not_merging(const Repository& repo);

    /**
     * Add all files in the repo directory into the index (excluding those in .gitignore)
     * and return the index tree.
     *
     * @param repo The repository.
     * @return The index tree.
     */
    Tree working_tree(const Repository& repo);

    /**
     * Finds differences between head and working dir index (must git add first).
     *
     * @param repo The repo to use for the HEAD.
     * @return The diff created between HEAD and working dir.
     */
    Diff current_changes(const Repository& repo);

    /**
     * Commit all files in the repo directory (excluding those in .gitignore) to updateRef.
     *
     * @param repo The repo to commit files to.
     * @param updateRef The reference to update to point to the new commit, e.g. "HEAD" to commit to the head of the current branch.
     * @param message Message to leave on the commit.
     * @param parentCommits The commit's parents.
     */
    void commit(const Repository& repo, const string& updateRef, const string& message, const vector<Commit>& parentCommits);

    /**
    * Commit all files in the repo directory (excluding those in .gitignore) to updateRef.
     *
    * @param repo The repo to commit files to.
    * @param updateRef The reference to update to point to the new commit, e.g. "HEAD" to commit to the head of the current branch.
    * @param message Message to leave on the commit.
    * @param parentRevs The revisions corresponding to the commit's parents.
    */
    void commit(const Repository& repo, const string& updateRef, const string& message, initializer_list<string> parentRevs);

    /**
     * Commit all files in the repo directory (excluding those in .gitignore) to the HEAD of the current branch.
     *
     * @param repo The repo to commit giles to the HEAD of the branch of.
     * @param message Message to leave on the commit.
     * @param parentCommits The commit's parents.
     */
    void commit(const Repository& repo, const string& message, const vector<Commit>& parentCommits);

    /**
     * Commit all files in the repo directory (excluding those in .gitignore) to the HEAD of the current branch.
     *
     * @param repo The repo to commit giles to the HEAD of the branch of.
     * @param message Message to leave on the commit.
     * @param parentCommits The revisions corresponding to the commit's parents.
     */
    void commit(const Repository& repo, const string& message, initializer_list<string> parentRevs);

    /**
     * Add all files from the working directory to the staging area.
     *
     * @param repo Repository to add files to.
     * @return The resulting index of adding the new files.
     */
    Index add_all(const Repository& repo);

    /**
     * Create a new empty git repository in the specified directory,
     * with an initial commit.
     *
     * @param path Path reference to creat new repo at (This should NOT contain the .git directory on the path).
     * @return Returns the repository created.
     * @throws RepositoryExistsException If the repo already exists.
     */
    Repository create(const string& path);

    /**
     * Deletes the commit at the HEAD of the current branch.
     *
     * @param repo Repository to delete the commit from.
     * @param reset True to make it a hard reset.
     * @throws UnsupportedOperationException If HEAD has no parents.
     */
    void delete_last_commit(const Repository& repo, bool reset);

    /**
     * Amends the last commit with your changes.
     * Note: THIS WILL REPLACE PREVIOUS COMMIT METADATA WITH YOUR OWN
     *
     * @param repo The repo to replace previous commit within.
     * @param message The new message to attach to the patched commit.
     * @throws CurrentlyMergingException if a merge is currently taking place.
     */
    void patch(const Repository& repo, const string& message);

    /**
     * Gets the commit corresponding to the given revision.
     *
     * @param repo Repo to find the commit in.
     * @param revision Revision reference of the commit to find.
     * @return The commit corresponding to given revision.
     */
    Commit get_commit(const Repository& repo, const string& revision);

    /**
     * Tests whether a certain commit exists.
     *
     * @param repo Repo to find the commit in.
     * @param name Revision name reference of the commit to find.
     * @return True if the commit exists
     */
    bool commit_exists(const Repository &repo, const string& name);

    /**
     * Create a new branch from the current head with the specified name.
     *
     * @param repo Repo to make the branch in.
     * @param name Name of the new branch to create.
     */
    void create_branch(const Repository &repo, const string& name);

    /**
     * Tests whether a certain branch exists in the repo.
     *
     * @param repo Repo to search for branch in.
     * @param name Branch name reference.
     * @return True if the branch exists.
     */
    bool branch_exists(const Repository &repo, const string& name);

    /**
     * Gets the current head of the repository.
     */
    Head get_head(const Repository& repo);

    /**
     * Checks if the repository's head is at the given branch name.
     * Always returns false if the head is detached.
     */
    bool is_on_branch(const Repository& repo, const string& branch);

    /**
     * Check if the current branch has any commits on it.
     */
    bool head_exists(const Repository& repo);

    /**
     * Deletes the branch of the given name.
     *
     * @param repo Repo to delete branch from.
     * @param name Name of branch to delete.
     * @throws UnsupportedOperationException If the branch to be deleted is the only non-WIP branch left.
     */
    void delete_branch(const Repository& repo, const string& name);

    /**
     * Checks out the given commit without moving head,
     * such that the working directory will match the commit contents.
     * Doesn't change current branch ref.
     *
     * @param repo Repo to checkout from.
     * @param name Name reference of ref to checkout.
     */
    void checkout(const Repository& repo, const string& name);

    /**
     * Checks out the given commit without moving head,
     * such that the working directory will match the commit contents.
     * Doesn't change current branch ref.
     *
     * @param repo Repo to checkout from.
     * @param commit Commit to checkout.
     */
    void checkout(const Repository& repo, const Commit& commit);

    /**
     * Whether the user has changes currently not committed.
     *
     * @param repo Repo to check against HEAD for.
     * @return True if there are uncommited changes.
     */
    bool has_uncommitted_changes(const Repository& repo);

    /**
     * Creates a list of conflicts for the given index.
     *
     * @param index Index used to create conflict list.
     * @return The list of conflicts in that index.
     */
    [[nodiscard]] vector<StandaloneConflict> get_conflicts(const Index& index);

    /**
     * If the working directory has changes since the last commit, or a merge has been started,
     * Save these changes in a WIP commit in a new #wip branch.
     *
     * @param repo Repo to save WIP for current branch in.
     */
    void save_wip(const Repository& repo);

    /**
     * Deletes the WIP commit at head if any, restoring the contents to the working directory
     * and resuming a merge if one was ongoing.
     *
     * @param repo Repo to restore WIP for.
     * @param force Whether to replace the current work.
     */
    void restore_wip(const Repository& repo, bool force);

    /**
     * Squashes the commits on the WIP branch into a single WIP commit, using the base
     * as the last commit on the current branch, and preserving and merges in the WIP.
     *
     * @param repo Repo to squash WIP in.
     */
    void squash_wip(const Repository& repo);

    /**
     * Moves to the given branch, checking out changes and the HEAD of that branch.
     *
     * @param repo Repo to switch to branch within.
     * @param name Name of branch to switch to.
     * @param saveWip Whether or not to save uncommitted changes to the WIP branch before switching.
     * @param restoreWip Whether or not to restore the WIP branch of the new branch after switching.
     * @throws UnsupportedOperationException If switching to a WIP branch is attempted.
     */
    void switch_branch(const Repository& repo, const string& name, bool saveWip, bool restoreWip);

    /**
     * Moves the head to the given ref.
     *
     * @param repo Repo to use for current and target head.
     * @param name Ref reference to switch to.
     */
    void move_head(const Repository& repo, const string& name);

    /**
     * Resets head to the specified commit.
     * If hard is specified the work dir is also reset to match the commit, otherwise it is left unmodified.
     */
    void reset_head(const Repository& repo, const Commit& commit, bool hard);

    /**
     * Fill a list with all the references that can be found in a repository.
     * Returns a string array filled with the names of all references.
     *
     * @param repo Repo to get list of references from.
     * @return Array of references.
     */
    StrArray reference_list(const Repository& repo);

    /**
     * Resets the current working directory to the empty tree - ie. an empty directory.
     * This does not wipe any data from the repository - only files in the working
     * directory or staging area.
     *
     * @param repo Repo to reset directory of
     */
    void reset_to_empty(const Repository& repo);

    /**
     * Iterates over a commit tree, running `pre` as a commit is entered and `post` as a commit is exited.
     * The first time either function returns true, the search will backtrack to the start without
     * exploring new commits.
     *
     * @param pre Function to run when entering nodes
     * @param post Function to run when exiting nodes
     * @param commit Root commit
     * @return True if the exit was forceful
     */
    bool tree_iterator(function<bool(Commit)> pre, function<bool(Commit)> post, Commit commit);
}
