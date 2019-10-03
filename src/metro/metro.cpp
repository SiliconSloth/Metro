#include "pch.h"

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
    bool mergeOngoing(const Repository& repo) {
        try {
            repo.revparse_single("MERGE_HEAD");
        } catch (exception& e) {
            return false;
        }
        return true;
    }

    void assertMerging(const Repository& repo) {
        if (mergeOngoing(repo)) {

        }
    }
}