#include "pch.h"
#define WIPString "#WIP"

using namespace git;

namespace metro {
    // Commit all files in the repo directory (excluding those in .gitignore) to the head of the current branch.
    // repo: The repo
    // message: The commit message
    // parentRevs: The revisions corresponding to the commit's parents
    void commit(const Repository& repo, const string& message, const initializer_list<string> parentRevs) {
        Signature author = repo.default_signature();

        Index index = repo.index();
        index.add_all({}, GIT_INDEX_ADD_DISABLE_PATHSPEC_MATCH, nullptr);
        // Write the files in the index into a tree that can be attached to the commit.
        OID oid = index.write_tree();
        Tree tree = repo.lookup_tree(oid);
        // Save the index to disk so that it stays in sync with the contents of the working directory.
        // If we don't do this removals of every file are left staged.
        index.write();

        // Retrieve the commit objects associated with the given parent revisions.
        vector<Commit> parentCommits;
        for (const string& parentRev : parentRevs) {
            parentCommits.push_back(reinterpret_cast<Commit>(repo.revparse_single(parentRev)));
        }

        // Commit the files to the head of the current branch.
        repo.create_commit("HEAD", author, author, "UTF-8", message, tree, parentCommits);
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
}