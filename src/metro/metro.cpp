#include "pch.h"

using namespace git;

namespace metro {
    void commit(const Repository& repo, const string& message, const initializer_list<string> parentRevs) {
        Signature author = repo.default_signature();

        Index index = repo.index();
        index.add_all({}, GIT_INDEX_ADD_DISABLE_PATHSPEC_MATCH, nullptr);
        OID oid = index.write_tree();
        Tree tree = repo.lookup_tree(oid);
        index.write();

        vector<Commit> parentCommits;
        for (const string& parentRev : parentRevs) {
            parentCommits.push_back(reinterpret_cast<Commit>(repo.revparse_single(parentRev)));
        }

        repo.create_commit("HEAD", author, author, "UTF-8", message, tree, parentCommits);
    }

    Repository create(const string& path) {
        if (Repository::exists(path+"/.git")) {
            throw RepositoryExistsException();
        }

        Repository repo = Repository::init(path+"/.git", false);
        commit(repo, "Create repository", {});
        return repo;
    }
}