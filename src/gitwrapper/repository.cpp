#include <gitwrapper/strarray.h>
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

    Repository Repository::clone(const string& url, const string& path, git_clone_options *options) {
        git_repository *gitRepo = nullptr;
        int err = git_clone(&gitRepo, url.c_str(), path.c_str(), options);
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

    string Repository::path() const {
        return string(git_repository_path(repo.get()));
    }

    Index Repository::index() const {
        git_index *index;
        int err = git_repository_index(&index, repo.get());
        check_error(err);
        return Index(index);
    }

    Tree Repository::lookup_tree(const OID &oid) const {
        git_tree *tree;
        int err = git_tree_lookup(&tree, repo.get(), &oid.oid);
        check_error(err);
        return Tree(tree);
    }

    Branch Repository::lookup_reference(const string& name) const {
        git_reference *ref;
        int err = git_reference_lookup(&ref, repo.get(), name.c_str());
        check_error(err);
        return Branch(ref);
    }

    Branch Repository::lookup_branch(const string &name, git_branch_t branchType) const {
        git_reference *branch;
        int err = git_branch_lookup(&branch, repo.get(), name.c_str(), branchType);
        check_error(err);
        return Branch(branch);
    }

    Commit Repository::lookup_commit(const OID& oid) const {
        git_commit *commit;
        int err = git_commit_lookup(&commit, repo.get(), &oid.oid);
        check_error(err);
        return Commit(commit);
    }

    AnnotatedCommit Repository::lookup_annotated_commit(const OID& id) const {
        git_annotated_commit *commit;
        int err = git_annotated_commit_lookup(&commit, repo.get(), &id.oid);
        check_error(err);
        return AnnotatedCommit(commit);
    }

    OID Repository::create_commit(const string& updateRef, const Signature &author, const Signature &committer,
                                  const string& messageEncoding, const string& message, const Tree& tree,
                                  vector<Commit> parents) const {
        auto parents_array = new const git_commit *[parents.size()];
        for (unsigned long long i = 0; i < parents.size(); i++) {
            parents_array[i] = parents[i].ptr().get();
        }

        git_oid id;
        int err = git_commit_create(&id, repo.get(), updateRef.c_str(), &author, &committer, messageEncoding.c_str(),
                                    message.c_str(), tree.ptr().get(), parents.size(), parents_array);
        delete[] parents_array;
        check_error(err);
        return OID(id);
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

    Branch Repository::create_reference(const string& name, const OID& oid, const bool force) const {
        git_reference *ref;
        int err = git_reference_create(&ref, repo.get(), name.c_str(), &oid.oid, force, nullptr);
        check_error(err);
        return Branch(ref);
    }

    void Repository::create_branch(const string& branch_name, const Commit &target, bool force) const {
        git_reference *ref;
        int err = git_branch_create(&ref, repo.get(), branch_name.c_str(), target.ptr().get(), force);
        check_error(err);
    }

    void Repository::remove_reference(const string& name) const {
        int err = git_reference_remove(repo.get(), name.c_str());
        check_error(err);
    }

    BranchIterator Repository::new_branch_iterator(const git_branch_t& flags) const {
        git_branch_iterator *iter;
        int err = git_branch_iterator_new(&iter, repo.get(), flags);
        check_error(err);
        return BranchIterator(iter);
    }

    void Repository::foreach_reference(const foreach_reference_cb& callback, const void *payload) const {
        foreach_reference_cb_payload cbp{callback, payload};
        int err = git_reference_foreach(repo.get(), [](git_reference *reference, void *payload) {
            auto *cb_payload = (foreach_reference_cb_payload*) payload;
            return cb_payload->callback(Branch(reference), cb_payload->payload);
        }, &cbp);
        check_error(err);
    }

    void Repository::foreach_reference_glob(const string& glob, const foreach_reference_glob_cb& callback, const void *payload) const {
        foreach_reference_glob_cb_payload cbp{callback, payload};
        int err = git_reference_foreach_glob(repo.get(), glob.c_str(), [](const char *name, void *payload) {
            auto *cb_payload = (foreach_reference_glob_cb_payload*) payload;
            return cb_payload->callback(string(name), cb_payload->payload);
        }, &cbp);
        check_error(err);
    }

    StatusList Repository::new_status_list(const git_status_options &options) const {
        git_status_list *status;
        int err = git_status_list_new(&status, repo.get(), &options);
        check_error(err);
        return StatusList(status);
    }

    void Repository::set_head(const string& name) const {
        int err = git_repository_set_head(repo.get(), name.c_str());
        check_error(err);
    }

    void Repository::checkout_tree(const Tree& tree, const git_checkout_options& options) const {
        int err = git_checkout_tree(repo.get(), reinterpret_cast<git_object*>(tree.ptr().get()), &options);
        check_error(err);
    }

    void Repository::cleanup_state() const {
        int err = git_repository_state_cleanup(repo.get());
        check_error(err);
    }

    git_merge_analysis_t Repository::merge_analysis(const vector<AnnotatedCommit>& sources) const {
        git_merge_analysis_t analysis;
        git_merge_preference_t preference;

        auto sources_array = new const git_annotated_commit*[sources.size()];
        for (size_t i = 0; i < sources.size(); i++) {
            sources_array[i] = sources[i].ptr().get();
        }

        int err = git_merge_analysis(&analysis, &preference, repo.get(), sources_array, sources.size());
        delete[] sources_array;
        check_error(err);
        return analysis;
    }

    void Repository::merge(const vector<AnnotatedCommit>& sources, const git_merge_options &merge_opts,
                           const git_checkout_options &checkout_opts) const {
        auto sources_array = new const git_annotated_commit*[sources.size()];
        for (size_t i = 0; i < sources.size(); i++) {
            sources_array[i] = sources[i].ptr().get();
        }

        int err = git_merge(repo.get(), sources_array, sources.size(), &merge_opts, &checkout_opts);
        delete[] sources_array;
        check_error(err);
    }

    StrArray Repository::remote_list() const {
        git_strarray array;
        int err = git_remote_list(&array, repo.get());
        check_error(err);

        return StrArray(&array);
    }

    Remote Repository::remote_create(string name, string url) const {
        git_remote *remote;
        int err = git_remote_create(&remote, repo.get(), name.c_str(), url.c_str());
        check_error(err);

        return Remote(remote);
    }

    void Repository::remote_set_url(string remote, string url) const {
        int err = git_remote_set_url(repo.get(), remote.c_str(), url.c_str());
        check_error(err);
    }

    Remote Repository::lookup_remote(string name) const {
        git_remote *remote;
        int err = git_remote_lookup(&remote, repo.get(), name.c_str());
        check_error(err);
        return Remote(remote);
    }

    OID Repository::merge_base(OID one, OID two) const {
        git_oid out;
        int err = git_merge_base(&out, repo.get(), &one.oid, &two.oid);
        check_error(err);
        return OID(out);
    }

    Config Repository::config() const {
        git_config *config;
        int err = git_repository_config(&config, repo.get());
        check_error(err);
        return Config(config);
    }
}