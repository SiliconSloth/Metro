using namespace git;

namespace metro {
    // Returns true if the repo is currently in merging state.
    bool merge_ongoing(const Repository& repo);

    void assert_merging(const Repository& repo);

    // Commit all files in the repo directory (excluding those in .gitignore) to updateRef.
    // updateRef: The reference to update to point to the new commit
    // repo: The repo
    // message: The commit message
    // parentCommits: The commit's parents
    Diff commit(const Repository& repo, const string& updateRef, const string& message, const vector<Commit>& parentCommits);

    // Commit all files in the repo directory (excluding those in .gitignore) to updateRef.
    // updateRef: The reference to update to point to the new commit
    // repo: The repo
    // message: The commit message
    // parentRevs: The revisions corresponding to the commit's parents
    Diff commit(const Repository& repo, const string& updateRef, const string& message, initializer_list<string> parentRevs);

    // Commit all files in the repo directory (excluding those in .gitignore) to the head of the current branch.
    // repo: The repo
    // message: The commit message
    // parentCommits: The commit's parents
    Diff commit(const Repository& repo, const string& message, const vector<Commit>& parentCommits);

    // Commit all files in the repo directory (excluding those in .gitignore) to the head of the current branch.
    // repo: The repo
    // message: The commit message
    // parentRevs: The revisions corresponding to the commit's parents
    Diff commit(const Repository& repo, const string& message, initializer_list<string> parentRevs);

    // Initialize an empty git repository in the specified directory,
    // with an initial commit.
    Repository create(const string& path);

    void delete_last_commit(const Repository& repo, bool reset);

    void patch(const Repository& repo, const string& message);

    // Gets the commit corresponding to the given revision
    // revision - Revision of the commit to find
    // repo - Repo to find the commit in
    //
    // Returns the commit
    Commit get_commit(const Repository& repo, const string& revision);

    bool commit_exists(const Repository &repo, const string& name);

    // Create a new branch from the current head with the specified name.
    // Returns the branch
    void create_branch(const Repository &repo, const string& name);

    bool branch_exists(const Repository &repo, const string& name);

    string current_branch_name(const Repository& repo);

    void delete_branch(const Repository& repo, const string& name);

    void checkout(const Repository& repo, const string& name);

    bool has_uncommitted_changes(const Repository& repo);

    [[nodiscard]] vector<StandaloneConflict> get_conflicts(const Index& index);

    void fast_forward(const Repository &repo, string name);

    // If the working directory has changes since the last commit, or a merge has been started,
    // Save these changes in a WIP commit in a new #wip branch.
    void save_wip(const Repository& repo);

    // Deletes the WIP commit at head if any, restoring the contents to the working directory
    // and resuming a merge if one was ongoing.
    void restore_wip(const Repository& repo);

    void switch_branch(const Repository& repo, const string& name);

    void move_head(const Repository& repo, const string& name);

    void checkout_branch(Repository repo, string name);
}