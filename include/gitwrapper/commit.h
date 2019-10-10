#pragma once

namespace git {
    class Commit {
    private:
        shared_ptr<git_commit> commit;

    public:
        explicit Commit(git_commit *commit) : commit(commit, git_commit_free) {}

        explicit Commit(shared_ptr<git_commit> commit) : commit(std::move(commit)) {}

        Commit() = delete;

        Commit operator=(Commit c) = delete;

        [[nodiscard]] shared_ptr<git_commit> ptr() const {
            return commit;
        }

        [[nodiscard]] string message() const;

        [[nodiscard]] OID id() const;

        [[nodiscard]] Tree tree() const;

        [[nodiscard]] unsigned int parentcount() const;

        [[nodiscard]] Commit parent(unsigned int n) const;

        [[nodiscard]] vector<Commit> parents() const;
    };
}