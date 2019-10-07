#include "pch.h"

namespace git {
    Repository Repository::init(const string& path, bool isBare) {
        git_repository *gitRepo = nullptr;
        int err = git_repository_init(&gitRepo, path.c_str(), isBare);
        check_error(err);

        return Repository(gitRepo);
    }

    Repository Repository::open(const string& path) {
        git_repository *gitRepo = nullptr;
        int err = git_repository_open(&gitRepo, path.c_str());
        check_error(err);

        return Repository(gitRepo);
    }

    bool Repository::exists(const string& path) {
        int err = git_repository_open_ext(nullptr, path.c_str(), GIT_REPOSITORY_OPEN_NO_SEARCH, nullptr);
        return err >= 0;
    }

    Signature &Repository::default_signature() const {
        git_signature *sig;
        int err = git_signature_default(&sig, repo.get());
        check_error(err);
        return *sig;
    }

    Index &Repository::index() const {
        git_index *index;
        int err = git_repository_index(&index, repo.get());
        check_error(err);
        Index out(index);
        return out;
    }

    Tree Repository::lookup_tree(const git_oid &oid) const {
        git_tree *tree;
        int err = git_tree_lookup(&tree, repo.get(), &oid);
        check_error(err);
        return tree;
    }

    Branch Repository::lookup_branch(const string &name, git_branch_t branchType) const {
        git_reference *branch;
        int err = git_branch_lookup(&branch, repo.get(), name.c_str(), branchType);
        check_error(err);
        return Branch(branch);
    }

    OID Repository::create_commit(const string& update_ref, const Signature &author, const Signature &committer,
                              const string& message_encoding, const string& message, const Tree& tree,
                              vector<Commit> parents) const {
        auto parents_array = new const git_commit *[parents.size()];
        for (unsigned long long i = 0; i < parents.size(); i++) {
            parents_array[i] = parents[i].ptr().get();
        }

        git_oid id;
        int err = git_commit_create(&id, repo.get(), update_ref.c_str(), &author, &committer, message_encoding.c_str(),
                                    message.c_str(), tree, parents.size(), parents_array);
        delete[] parents_array;
        check_error(err);
        return id;
    }

    Object Repository::revparse_single(const string& spec) const {
        git_object *obj;
        int err = git_revparse_single(&obj, repo.get(), spec.c_str());
        check_error(err);
        return Object(obj);
    }

    void Repository::reset_to_commit(const Commit &commit, ResetType type, const CheckoutOptions ops) const {
        int err = git_reset(repo.get(), (git_object*) commit.ptr().get(), type, &ops);
        check_error(err);
    }

    StatusList &Repository::status_list_new(const git_status_options ops) const {
        git_status_list *status;
        int err = git_status_list_new(&status, repo.get(), &ops);
        check_error(err);
        return *status;
    }

    void Repository::create_branch(string branch_name, Commit &target, bool force) {
        git_reference *ref;
        int err = git_branch_create(&ref, repo.get(), branch_name.c_str(), target, force);
        check_error(err);
    }

    void Repository::branch_lookup(string branch_name, bool isLocal) {
        git_reference *ref;
        int err = git_branch_lookup(&ref, repo.get(), branch_name.c_str(), GIT_BRANCH_LOCAL);
        check_error(err);
    }

    BranchIterator Repository::new_branch_iterator(const git_branch_t& flags) const {
        git_branch_iterator *iter;
        int err = git_branch_iterator_new(&iter, repo.get(), flags);
        check_error(err);
        return BranchIterator(iter);
    }
}