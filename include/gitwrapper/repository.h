#pragma once

namespace git {
    class Repository {
    private:
        explicit Repository(git_repository *repo) : repo(repo, git_repository_free) {}

        shared_ptr<git_repository> repo;

    public:
        Repository() = delete;

        [[nodiscard]] shared_ptr<git_repository> ptr() const {
            return repo;
        }

        static Repository init(const string& path, bool isBare);
        static Repository open(const string& path);
        static bool exists(const string& path);

        [[nodiscard]] Signature &default_signature() const;
        [[nodiscard]] Index &index() const;
        [[nodiscard]] Tree lookup_tree(const OID &oid) const;
        [[nodiscard]] Branch lookup_branch(const string& name, git_branch_t branchType) const;
        OID create_commit(const string& update_ref, const Signature &author, const Signature &committer,
                              const string& message_encoding, const string& message, const Tree& tree,
                              vector<Commit> parents) const;
        Object revparse_single(const string& spec) const;
        void reset_to_commit(const Commit &, ResetType, CheckoutOptions) const;

        void create_branch(const string& branch_name, Commit &target, bool force);

        void branch_lookup(const string& branch_name, bool isLocal);

        [[nodiscard]] BranchIterator new_branch_iterator(const git_branch_t& flags) const;
    };

}