#pragma once

namespace git {
    class Diff {
    private:
        shared_ptr<git_diff> diff;

    public:
        explicit Diff(git_diff *diff) : diff(diff, git_diff_free) {}

        explicit Diff(shared_ptr<git_diff> diff) : diff(std::move(diff)) {}

        Diff() = delete;

        Diff operator=(Diff d) = delete;

        [[nodiscard]] shared_ptr<git_diff> ptr() const {
            return diff;
        }

        size_t num_deltas();

        static Diff tree_to_tree(const Repository& repo, const Tree& oldTree, const Tree& newTree,
                const git_diff_options* opts);
    };
}