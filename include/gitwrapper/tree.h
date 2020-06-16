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
        Tree() : tree(nullptr) {}

        explicit Tree(git_tree *tree) : tree(tree, git_tree_free) {}

        explicit Tree(shared_ptr<git_tree> tree) : tree(std::move(tree)) {}

        [[nodiscard]] shared_ptr<git_tree> ptr() const {
            return tree;
        }

        /**
         * Get the id of a tree.
         *
         * @return OID of the tree.
         */
        [[nodiscard]] OID id() const;
    };
}