namespace git {
    string Commit::message() const {
        return string(git_commit_message(commit.get()));
    }

    OID Commit::id() const {
        return OID(*git_commit_id(commit.get()));
    }

    Tree Commit::tree() const {
        git_tree *tree;
        int err = git_commit_tree(&tree, commit.get());
        check_error(err);
        return Tree(tree);
    }

    unsigned int Commit::parentcount() const {
        return git_commit_parentcount(commit.get());
    }

    Commit Commit::parent(unsigned int n) const {
        git_commit *parent;
        int err = git_commit_parent(&parent, commit.get(), n);
        check_error(err);
        return Commit(parent);
    }

    vector<Commit> Commit::parents() const {
        vector<Commit> parents;
        unsigned int count = parentcount();
        for (unsigned int i = 0; i < count; i++) {
            parents.push_back(parent(i));
        }
        return parents;
    }

    OID Commit::parentID(int n) const {
        const git_oid* oid = git_commit_parent_id(commit.get(), n);
        return OID(*oid);
    }

    git_signature Commit::author() const {
        const git_signature* sig = git_commit_author(commit.get());
        return *sig;
    }

    OID Commit::amend(const string& updateRef, const git_signature& author, const git_signature& committer,
              const string& messageEncoding, const string& message, const Tree& tree) const {
        git_oid oid;
        int err = git_commit_amend(&oid, commit.get(), updateRef.c_str(), &author, &committer,
                messageEncoding.c_str(), message.c_str(), tree.ptr().get());
        check_error(err);
        return OID(oid);
    }
}