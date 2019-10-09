#define WIPString "#WIP"

using namespace git;

namespace metro {
    // Returns true if the repo is currently in merging state.
    bool merge_ongoing(const Repository& repo);

    void assert_merging(const Repository& repo);

    // Commit all files in the repo directory (excluding those in .gitignore) to the head of the current branch.
    // repo: The repo
    // message: The commit message
    // parentCommits: The commit's parents
    void commit(const Repository& repo, const string& message, const vector<Commit>& parentCommits);

    // Commit all files in the repo directory (excluding those in .gitignore) to the head of the current branch.
    // repo: The repo
    // message: The commit message
    // parentRevs: The revisions corresponding to the commit's parents
    void commit(const Repository& repo, const string& message, const initializer_list<string> parentRevs);

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

    // Create a new branch from the current head with the specified name.
    // Returns the branch
    void create_branch(Repository &repo, const string& name);

    bool branch_exists(Repository &repo, const string& name);

    string current_branch_name(const Repository& repo);

    void delete_branch(const Repository& repo, const string& name);

    bool has_uncommitted_changes(const Repository& repo);

    void save_wip(Repository& repo);
}