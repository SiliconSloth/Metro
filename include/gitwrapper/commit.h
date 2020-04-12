/*
 * Wrapper for the git_commit type.
 */

#pragma once

namespace git {
    /**
     * Parsed representation of a commit object.
     */
    class Commit {
    private:
        shared_ptr<git_commit> commit;

    public:
        explicit Commit(git_commit *commit) : commit(commit, git_commit_free) {}

        explicit Commit(shared_ptr<git_commit> commit) : commit(std::move(commit)) {}

        Commit() = delete;

        Commit operator=(Commit c) = delete;

        [[nodiscard]] shared_ptr<git_commit> ptr() const {
            return commit;
        }

        /**
         * Get the full message of a commit.
         *
         * The returned message will be slightly prettified by removing any potential leading newlines.
         *
         * @return Message associated with commit.
         */
        [[nodiscard]] string message() const;

        /**
         * Get the id of a commit.
         *
         * @return OID of the commit.
         */
        [[nodiscard]] OID id() const;

        /**
         * Get the tree pointed to by a commit.
         *
         * @return Commit as a tree.
         */
        [[nodiscard]] Tree tree() const;

        /**
         * Get the number of parents of this commit
         *
         * @return Number of parents of the commit.
         */
        [[nodiscard]] unsigned int parentcount() const;

        /**
         * Get the specified parent of the commit.
         *
         * @param n Which parent to get.
         * @return The commit parent.
         */
        [[nodiscard]] Commit parent(unsigned int n) const;

        /**
         * Gets a vector of all parent commits.
         *
         * @return Vector of all parents.
         */
        [[nodiscard]] vector<Commit> parents() const;

        /**
         * Get the oid of a specified parent for a commit. This is different from `Commit::parent`,
         * which will attempt to load the parent commit from the ODB.
         *
         * @param n Which parent to get.
         * @return OID of nth parent.
         */
        [[nodiscard]] OID parentID(int n) const;

        /**
         * Gets the author of the commit.
         *
         * @return Commit author as a Signature.
         */
        [[nodiscard]] Signature author() const;
    };
}