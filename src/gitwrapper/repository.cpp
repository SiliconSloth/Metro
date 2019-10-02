#include "pch.h"

namespace git {

    Repository::~Repository() {
        git_repository_free(repo);
    }

    Repository Repository::init(string path, bool isBare) {
        git_repository *gitRepo = nullptr;
        int err = git_repository_init(&gitRepo, path.c_str(), isBare);
        check_error(err);

        return Repository(gitRepo);
    }

    Repository Repository::open(string path) {
        git_repository *gitRepo = nullptr;
        int err = git_repository_open(&gitRepo, path.c_str());
        check_error(err);

        return Repository(gitRepo);
    }

    bool Repository::exists(string path) {
        int err = git_repository_open_ext(nullptr, path.c_str(), GIT_REPOSITORY_OPEN_NO_SEARCH, nullptr);
        return err >= 0;
    }

    Signature &Repository::default_signature() const {
        git_signature *sig;
        int err = git_signature_default(&sig, repo);
        check_error(err);
        return *sig;
    }

    Index &Repository::index() const {
        git_index *index;
        int err = git_repository_index(&index, repo);
        check_error(err);
        Index out(index);
        return out;
    }

    Tree Repository::lookup_tree(const git_oid &oid) const {
        git_tree *tree;
        int err = git_tree_lookup(&tree, repo, &oid);
        check_error(err);
        return tree;
    }

    OID Repository::create_commit(string update_ref, const Signature &author, const Signature &committer,
                              string message_encoding, string message, const Tree tree,
                              vector<Commit> parents) const {
        git_commit *commit;
        const git_commit **parents_array = new const git_commit *[parents.size()];
        copy(parents.begin(), parents.end(), parents_array);
        git_oid id;
        int err = git_commit_create(&id, repo, update_ref.c_str(), &author, &committer, message_encoding.c_str(),
                                    message.c_str(), tree, 0, parents_array);
        check_error(err);
        return id;
    }

    Object &Repository::revparse_single(string spec) const {
        git_object *obj;
        int err = git_revparse_single(&obj, repo, spec.c_str());
        check_error(err);
        return obj;
    }

    void Repository::reset_to_commit(const Commit &commit, ResetType type, const CheckoutOptions ops) const {
        int err = git_reset(repo, (git_object *) &commit, type, &ops);
        check_error(err);
    }

    StatusList &Repository::status_list_new(const git_status_options ops) const {
        git_status_list *status;
        int err = git_status_list_new(&status, repo, &ops);
        check_error(err);
        return *status;
    }

}