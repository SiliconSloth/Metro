/*
 * Contains a wrapper for the git_tree object.
 */

#pragma once

namespace git {
    /**
     * Representation of a tree object.
     */
    class Tree {
    private:
        shared_ptr<git_tree> tree;

    public:
        explicit Tree(git_tree *tree) : tree(tree, git_tree_free) {}

        explicit Tree(shared_ptr<git_tree> tree) : tree(std::move(tree)) {}

        Tree() = delete;

        Tree operator=(Tree t) = delete;

        [[nodiscard]] shared_ptr<git_tree> ptr() const {
            return tree;
        }
    };
}