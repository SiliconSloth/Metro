#include "pch.h"

namespace git {
    string Commit::message() const {
        return string(git_commit_message(commit.get()));
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
}