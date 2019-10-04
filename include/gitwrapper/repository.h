#pragma once

namespace git {
    class Repository {
    private:
        explicit Repository(git_repository *repo) : repo(repo, git_repository_free) {}

        shared_ptr<git_repository> repo;

    public:
        Repository() = delete;

        Repository operator=(Repository r) = delete;

        static Repository init(const string& path, bool isBare);
        static Repository open(const string& path);
        static bool exists(const string& path);

        [[nodiscard]] Signature &default_signature() const;
        [[nodiscard]] Index &index() const;
        [[nodiscard]] Tree lookup_tree(const OID &oid) const;
        [[nodiscard]] OID create_commit(const string& update_ref, const Signature &author, const Signature &committer,
                              const string& message_encoding, const string& message, const Tree& tree,
                              vector<Commit> parents) const;
        [[nodiscard]] Object &revparse_single(const string& spec) const;
        void reset_to_commit(const Commit &, ResetType, CheckoutOptions) const;
        [[nodiscard]]StatusList &status_list_new(StatusOptions) const;

        void create_branch(string branch_name, Commit &target, bool force);

        void branch_lookup(string branch_name, bool isLocal);
    };

}