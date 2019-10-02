#pragma once

namespace git {
    class Repository {
    private:
        Repository(git_repository *repo) : repo(repo) {}

        git_repository *repo;

    public:
        Repository() = delete;
        ~Repository();

        Repository operator=(Repository r) = delete;

        static Repository init(string path, bool isBare);
        static Repository open(string path);
        static bool exists(string path);

        Signature &default_signature();
        Index &index();
        Tree lookup_tree(const OID &oid);
        OID create_commit(string update_ref, const Signature &author, const Signature &committer,
                              string message_encoding, string message, const Tree tree,
                              vector<Commit *> parents);
        Object &revparse_single(string spec);
        void reset_to_commit(const Commit &, ResetType, CheckoutOptions);
        StatusList &status_list_new(StatusOptions);
    };

}