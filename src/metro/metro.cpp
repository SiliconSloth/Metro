#include "pch.h"
#define WIPString "#WIP"

using namespace git;

namespace metro {
    // Returns true if the repo is currently in merging state.
    bool merge_ongoing(const Repository& repo) {
        try {
            repo.revparse_single("MERGE_HEAD");
        } catch (exception& e) {
            return false;
        }
        return true;
    }

    void assert_merging(const Repository& repo) {
        if (merge_ongoing(repo)) {
            throw CurrentlyMergingException();
        }
    }

    // Commit all files in the repo directory (excluding those in .gitignore) to the head of the current branch.
    // repo: The repo
    // message: The commit message
    // parentCommits: The commit's parents
    void commit(const Repository& repo, const string& message, const vector<Commit> parentCommits) {
        Signature author = repo.default_signature();

        Index index = repo.index();
        index.add_all({}, GIT_INDEX_ADD_DISABLE_PATHSPEC_MATCH, nullptr);
        // Write the files in the index into a tree that can be attached to the commit.
        OID oid = index.write_tree();
        Tree tree = repo.lookup_tree(oid);
        // Save the index to disk so that it stays in sync with the contents of the working directory.
        // If we don't do this removals of every file are left staged.
        index.write();

        // Commit the files to the head of the current branch.
        repo.create_commit("HEAD", author, author, "UTF-8", message, tree, parentCommits);
    }

    // Commit all files in the repo directory (excluding those in .gitignore) to the head of the current branch.
    // repo: The repo
    // message: The commit message
    // parentRevs: The revisions corresponding to the commit's parents
    void commit(const Repository& repo, const string& message, const initializer_list<string> parentRevs) {
        // Retrieve the commit objects associated with the given parent revisions.
        vector<Commit> parentCommits;
        for (const string& parentRev : parentRevs) {
            parentCommits.push_back(static_cast<Commit>(repo.revparse_single(parentRev)));
        }

        commit(repo, message, parentCommits);
    }

    // Initialize an empty git repository in the specified directory,
    // with an initial commit.
    Repository create(const string& path) {
        if (Repository::exists(path+"/.git")) {
            throw RepositoryExistsException();
        }

        Repository repo = Repository::init(path+"/.git", false);
        commit(repo, "Create repository", {});
        return repo;
    }

    void delete_last_commit(const Repository& repo, bool reset) {
        Commit parent = static_cast<Commit>(repo.revparse_single("HEAD")).parent(0);

        git_checkout_options checkoutOpts = GIT_CHECKOUT_OPTIONS_INIT;
        checkoutOpts.checkout_strategy = GIT_CHECKOUT_FORCE;
        git_reset_t resetType = reset? GIT_RESET_HARD : GIT_RESET_SOFT;

        repo.reset_to_commit(parent, resetType, checkoutOpts);
    }

    void patch(const Repository& repo, const string& message) {
        assert_merging(repo);
        vector<Commit> parents = static_cast<Commit>(repo.revparse_single("HEAD")).parents();
        delete_last_commit(repo, false);
        commit(repo, message, parents);
    }

    // Gets the commit corresponding to the given revision
    // revision - Revision of the commit to find
    // repo - Repo to find the commit in
    //
    // Returns the commit
    Commit get_commit(string revision, Repository repo) {
        Object object = repo.revparse_single(revision);
        Commit commit = (Commit) object;
        return commit;
    }

    // Create a new branch from the current head with the specified name.
    // Returns the branch
    void create_branch(string name, Repository &repo) {
        Commit commit = get_commit("HEAD", repo);
        repo.create_branch(name, commit, false);
    }

    bool branch_exists(Repository &repo, string name) {
        try {
            repo.branch_lookup(name, true);
            return true;
        } catch (GitException &e) {
            return false;
        }
    }

    int transfer_progress_callback(const git_transfer_progress *stats, void *) {
        int progress = 100 * (stats->received_objects + stats->indexed_objects) / (stats->total_objects);
        printf("\rProgress: %d%%", progress);
        if (progress == 100) {
            cout << endl;
        }
        return GIT_OK;
    }

    int credentials_callback(git_cred **out, const char *url, const char *username_from_url, unsigned int allowed_types, void *) {
        string username;
        string password;
        switch (allowed_types) {
            case GIT_CREDTYPE_DEFAULT:
                git_cred_default_new(out);
                break;
            case GIT_CREDTYPE_USERPASS_PLAINTEXT:
                cout << "Username: ";
                cin >> username;

                cout << "Password: ";
                cin >> password;

                git_cred_userpass_plaintext_new(out, username.c_str(), password.c_str());
                break;
            default:
                cout << "Metro currently doesn't support SSH. Please use HTTPS.";
                return GIT_ERROR;
        }
        return GIT_OK;
    }

    Callbacks create_callbacks() {
        Callbacks callbacks;
        callbacks.transfer_progress = transfer_progress_callback;
        callbacks.credentials = credentials_callback;
        return callbacks;
    }
}