/*
 * Defines a wrapper for git_diff.
 */

#pragma once

namespace git {
    /**
     * The diff object that contains all individual file deltas.
     *
     * A `diff` represents the cumulative list of differences between two snapshots of a repository
     * (possibly filtered by a set of file name patterns).
     *
     * Calculating diffs is generally done in two phases: building a list of diffs then traversing it.
     * This makes is easier to share logic across the various types of diffs (tree vs tree, workdir vs
     * index, etc.), and also allows you to insert optional diff post-processing phases, such as rename
     * detection, in between the steps.
     */
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

        /**
         * Query how many diff records are there in a diff.
         *
         * @return The number of deltas in the diff
         */
        size_t num_deltas();

        /**
         * Query how many diff deltas are there in a diff filtered by type.
         * This works just like `git_diff_entrycount()` with an extra parameter that is a
         * `git_delta_t` and returns just the count of how many deltas match that particular type.
         *
         * @param type Type of delta to search for.
         * @return The number of deltas of the given type.
         */
        size_t num_deltas_of_type(git_delta_t);

        /**
         * Create a diff with the difference between two tree objects.
         * This is equivalent to `git diff`
         *
         * The first tree will be used for the "old_file" side of the delta and the second tree will
         * be used for the "new_file" side of the delta. You can pass NULL to indicate an empty tree,
         * although it is an error to pass NULL for both the `old_tree` and `new_tree`.
         *
         * @param repo Repo to compare trees of.
         * @param oldTree First tree to compare.
         * @param newTree Second tree to compare.
         * @param opts Options to use in comparison.
         * @return Diff between two selected trees.
         */
        static Diff tree_to_tree(const Repository& repo, const Tree& oldTree, const Tree& newTree,
                const git_diff_options* opts);

        /**
         * Create a diff between a tree and the working directory.
         *
         * The tree you provide will be used for the "old_file" side of the delta,
         * and the working directory will be used for the "new_file" side.
         *
         * This is not the same as `git diff` or `git diff-index`. Those commands use
         * information from the index, whereas this function strictly returns the differences
         * between the tree and the files in the working directory, regardless of the state of
         * the index. Use `git_diff_tree_to_workdir_with_index` to emulate those commands.
         *
         * To see difference between this and `git_diff_tree_to_workdir_with_index`, consider
         * the example of a staged file deletion where the file has then been put back into the
         * working dir and further modified. The tree-to-workdir diff for that file is 'modified',
         * but `git diff` would show status 'deleted' since there is a staged delete.
         *
         * @param repo Repo to compare trees of.
         * @param oldTree Old tree to compare to workdir.
         * @param opts Options to use in comparison.
         * @return Diff between tree and workdir.
         */
        static Diff tree_to_workdir(const Repository& repo, const Tree& oldTree, const git_diff_options* opts);

        /**
         * Create a diff between a tree and the working directory using index data to account
         * for staged deletes, tracked files, etc.
         *
         * This emulates `git diff  ` by diffing the tree to the index and the index to the
         * working directory and blending the results into a single diff that includes staged deleted, etc.
         *
         * @param repo Repo to compare trees of.
         * @param oldTree Old tree to compare to workdir.
         * @param opts Options to use in comparison.
         * @return Diff between tree and workdir.
         */
        static Diff tree_to_workdir_with_index(const Repository& repo, const Tree& oldTree, const git_diff_options* opts);
    };
}