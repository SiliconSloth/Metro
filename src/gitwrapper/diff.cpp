namespace git {
    size_t Diff::num_deltas() {
        return git_diff_num_deltas(diff.get());
    }

    size_t Diff::num_deltas_of_type(git_delta_t type) {
        return git_diff_num_deltas_of_type(diff.get(), type);
    }

    Diff Diff::tree_to_tree(const Repository& repo, const Tree& oldTree, const Tree& newTree,
            const git_diff_options* opts) {

        git_diff *diff;
        int err = git_diff_tree_to_tree(&diff, repo.ptr().get(), oldTree.ptr().get(), newTree.ptr().get(), opts);
        check_error(err);
        return Diff(diff);
    }

    Diff Diff::tree_to_workdir(const Repository& repo, const Tree& oldTree, const git_diff_options* opts) {
        git_diff* diff;
        int err = git_diff_tree_to_workdir(&diff, repo.ptr().get(), oldTree.ptr().get(), opts);
        check_error(err);
        return Diff(diff);
    }

    Diff Diff::tree_to_workdir_with_index(const Repository& repo, const Tree& oldTree, const git_diff_options* opts) {
        git_diff* diff;
        int err = git_diff_tree_to_workdir_with_index(&diff, repo.ptr().get(), oldTree.ptr().get(), opts);
        check_error(err);
        return Diff(diff);
    }
}