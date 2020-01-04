#include "pch.h"

namespace git {
    size_t Diff::num_deltas() {
        return git_diff_num_deltas(diff.get());
    }

    Diff Diff::tree_to_tree(const Repository& repo, const Tree& oldTree, const Tree& newTree,
            const git_diff_options* opts) {

        git_diff *diff;
        int err = git_diff_tree_to_tree(&diff, repo.ptr().get(), oldTree.ptr().get(), newTree.ptr().get(), opts);
        check_error(err);
        return Diff(diff);
    }
}