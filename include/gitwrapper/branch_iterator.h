/*
 * Describes a wrapper for the git_branch_iterator type.
 */

#pragma once

namespace git {
    /**
     * Iterator type for branches.
     */
    class BranchIterator {
    private:
        shared_ptr<git_branch_iterator> iter;

    public:
        explicit BranchIterator(git_branch_iterator *iter) : iter(iter, git_branch_iterator_free) {}

        BranchIterator() = delete;

        [[nodiscard]] shared_ptr<git_branch_iterator> ptr() {
            return iter;
        }

        /**
         * Next value in the branch iterator.
         *
         * @param branch Branch pointer to return next branch.
         * @return True if there was a branch returned.
         */
        bool next(Branch *branch) const;
    };
}