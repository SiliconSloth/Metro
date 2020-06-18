/*
 * Defines a wrapper for the git_treebuilder type.
 */

#pragma once

namespace git {
    /**
     * In-memory representation of a treebuilder.
     */
    class Treebuilder {
    private:
        shared_ptr<git_treebuilder> tb;

    public:
        explicit Treebuilder(git_treebuilder *tb) : tb(tb, git_treebuilder_free) {}

        Treebuilder() = default;

        [[nodiscard]] shared_ptr<git_treebuilder> ptr() const {
            return tb;
        }

        /**
         * Create a new tree builder.
         *
         * The tree builder can be used to create or modify trees in memory and
         * write them as tree objects to the database.
         *
         * The tree builder will start with no
         * entries and will have to be filled manually.
         *
         * @param repo Repository in which to store the object
         * @return Created Treebuilder
         */
        static Treebuilder create(const Repository &repo);
        /**
         * Create a new tree builder.
         *
         * The tree builder can be used to create or modify trees in memory and
         * write them as tree objects to the database.
         *
         * The tree builder will be
         * initialized with the entries of the given tree.
         *
         * @param repo Repository in which to store the object
         * @param tree Source tree to initialize the builder
         * @return Created Treebuilder
         */
        static Treebuilder create(const Repository &repo, Tree tree);

        /**
         * Write the contents of the tree builder as a tree object
         *
         * The tree builder will be written to the given `repo`, and its
         * identifying SHA1 hash will be returned.
         *
         * @return OID for tree built.
         */
        OID write();
    };
}