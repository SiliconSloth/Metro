#pragma once

namespace git {
    class BranchIterator {
    private:
        shared_ptr<git_branch_iterator> iter;
        Branch lastItem;
        bool cached = false;

    public:
        explicit BranchIterator(git_branch_iterator *iter) : iter(iter, git_branch_iterator_free) {}

        BranchIterator() = delete;

        [[nodiscard]] shared_ptr<git_branch_iterator> ptr() {
            return iter;
        }

        bool next(Branch *branch) const;
    };
}