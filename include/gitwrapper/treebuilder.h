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

        static Treebuilder create(const Repository &repo);
        static Treebuilder create(const Repository &repo, Tree tree);

        OID write();
    };
}