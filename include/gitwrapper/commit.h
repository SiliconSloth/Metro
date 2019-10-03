#pragma once

namespace git {
    class Commit {
    private:
        explicit Commit(git_commit *commit) : commit(commit, git_commit_free) {}

        shared_ptr<git_commit> commit;

    public:
        Commit() = delete;

        Commit operator=(Commit c) = delete;
    };
}