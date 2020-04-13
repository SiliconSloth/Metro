/*
 * Data structures for iterating over merge conflicts.
 */

#pragma once

#define DEREF_ENTRY(ptr) ptr == NULL? git_index_entry{} : *ptr
#define DEREF_PATH(ptr) ptr == NULL? string() : string(ptr->path)
#define REPLACE_PTR(old, rep) old == NULL? NULL : rep

namespace git {
    /**
     * Data structure representing a conflict path between two branches.
     */
    struct Conflict {
        const git_index_entry *ancestor;
        const git_index_entry *ours;
        const git_index_entry *theirs;

        Conflict() = default;

        Conflict(const git_index_entry *ancestor, const git_index_entry *ours, const git_index_entry *theirs):
            ancestor(ancestor), ours(ours), theirs(theirs)
        {}
    };

    /**
     * Conflict that contains the backing data within itself, rather than referencing memory controlled by a git index.
     */
    class StandaloneConflict : public Conflict {
        git_index_entry ancestor_val;
        git_index_entry ours_val;
        git_index_entry theirs_val;

        string ancestor_path;
        string ours_path;
        string theirs_path;

    public:
        explicit StandaloneConflict(const Conflict& conflict):
            ancestor_val(DEREF_ENTRY(conflict.ancestor)), ours_val(DEREF_ENTRY(conflict.ours)), theirs_val(DEREF_ENTRY(conflict.theirs)),
            ancestor_path(DEREF_PATH(conflict.ancestor)), ours_path(DEREF_PATH(conflict.ours)), theirs_path(DEREF_PATH(conflict.theirs)),
            Conflict(REPLACE_PTR(conflict.ancestor, &ancestor_val), REPLACE_PTR(conflict.ours, &ours_val), REPLACE_PTR(conflict.theirs, &theirs_val))
        {
            ancestor_val.path = ancestor_path.c_str();
            ours_val.path = ours_path.c_str();
            theirs_val.path = theirs_path.c_str();
        }

        StandaloneConflict(const StandaloneConflict& conflict):
            StandaloneConflict(*reinterpret_cast<const Conflict*>(&conflict))
        {}
    };

    /**
     * An iterator for conflicts in the index.
     */
    class ConflictIterator {
    private:
        shared_ptr<git_index_conflict_iterator> iter;
        Branch lastItem;
        bool cached = false;

    public:
        explicit ConflictIterator(git_index_conflict_iterator *iter) : iter(iter, git_index_conflict_iterator_free) {}

        ConflictIterator() = delete;

        [[nodiscard]] shared_ptr<git_index_conflict_iterator> ptr() {
            return iter;
        }

        /**
         * Gets the current conflict (ancestor, ours and theirs entry) and advance the iterator internally to the
         * next value.
         * @param out The next conflict found.
         * @return True if there was another conflict found.
         */
        bool next(Conflict &out) const;
    };

}