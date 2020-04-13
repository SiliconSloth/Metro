/*
 * Contains wrapper for git_index.
 */

#pragma once

namespace git {

    /**
     * Memory representation of an index file.
     */
    class Index {
    private:
        shared_ptr<git_index> index;

    public:
        explicit Index(git_index *index) : index(index, git_index_free) {};

        Index() = delete;

        Index operator=(Index i) = delete;

        [[nodiscard]] shared_ptr<git_index> ptr() const {
            return index;
        }

        /**
         * Add or update index entries matching files in the working directory.
         *
         * This method will fail in bare index instances.
         *
         * The `pathspec` is a list of file names or shell glob patterns that will
         * be matched against files in the repository's working directory. Each file
         * that matches will be added to the index (either updating an existing entry
         * or adding a new entry). You can disable glob expansion and force exact
         * matching with the `GIT_INDEX_ADD_DISABLE_PATHSPEC_MATCH` flag.
         *
         * Files that are ignored will be skipped (unlike `git_index_add_bypath`). If
         * a file is already tracked in the index, then it *will* be updated even if it
         * is ignored. Pass the `GIT_INDEX_ADD_FORCE` flag to skip the checking of ignore
         * rules.
         *
         * To emulate `git add -A` and generate an error if the pathspec contains the
         * exact path of an ignored file (when not using FORCE), add the
         * `GIT_INDEX_ADD_CHECK_PATHSPEC` flag. This checks that each entry in the
         * `pathspec` that is an exact match to a filename on disk is either not
         * ignored or already in the index. If this check fails, the function will
         * return GIT_EINVALIDSPEC.
         *
         * To emulate `git add -A` with the "dry-run" option, just use a callback
         * function that always returns a positive value. See below for details.
         *
         * If any files are currently the result of a merge conflict, those files
         * will no longer be marked as conflicting. The data about the conflicts
         * will be moved to the "resolve undo" (REUC) section.
         *
         * If you provide a callback function, it will be invoked on each matching
         * item in the working directory immediately *before* it is added to / updated
         * in the index. Returning zero will add the item to the index, greater than
         * zero will skip the item, and less than zero will abort the scan and return
         * that value to the caller.
         *
         * @param pathspec Array of path patterns.
         * @param flags Combination of git_index_add_option_t flags.
         * @param callback Notification callback for each added/updated path
         * (also gets index of matching pathspec entry); can be NULL; return 0 to add,
         * >0 to skip, 0 to abort scan.
         */
        void add_all(const StrArray& pathspec, unsigned int flags, git_index_matched_path_cb callback);

        /**
         * Write the index as a tree
         *
         * This method will scan the index and write a representation
         * of its current state back to disk; it recursively creates
         * tree objects for each of the subtrees stored in the index,
         * but only returns the OID of the root tree. This is the OID
         * that can be used e.g. to create a commit.
         *
         * The index instance cannot be bare, and needs to be associated
         * to an existing repository.
         *
         * The index must not contain any file in conflict.
         *
         * @return OID of written tree.
         */
        OID write_tree();

        /**
         * Write an existing index object from memory back to disk
         * using an atomic file lock.
         */
        void write();

        /**
         * Create an iterator for the conflicts in the index.
         *
         * The index must not be modified while iterating; the results are undefined.
         *
         * @return The newly created conflict iterator
         */
        [[nodiscard]] ConflictIterator conflict_iterator() const;

        /**
         * Get the count of entries currently in the index
         *
         * @return Integer of count of current entries.
         */
        [[nodiscard]] size_t entrycount() const;

        /**
         * Add or update index entries to represent a conflict.  Any staged
         * entries that exist at the given paths will be removed.
         *
         * The entries are the entries from the tree included in the merge.
         *
         * @param conflict Summary conflict to add.
         */
        void add_conflict(const Conflict& conflict) const;

        /**
         * Remove all conflicts in the index (entries with a stage greater than 0).
         */
        void cleanup_conflicts() const;

        /**
         * Determine if the index contains entries representing file conflicts.
         *
         * @return True if at least one conflict is found
         */
        [[nodiscard]] bool has_conflicts() const;
    };

}