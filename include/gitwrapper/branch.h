/*
 * Defines the git_reference wrapper for branches.
 */

#pragma once

namespace git {
    /**
     * In-memory representation of a branch.
     */
    class Branch {
    private:
        shared_ptr<git_reference> ref;

    public:
        explicit Branch(git_reference *ref) : ref(ref, git_reference_free) {}

        Branch() = default;

        [[nodiscard]] shared_ptr<git_reference> ptr() const {
            return ref;
        }

        /**
         * Gets the branch name.
         *
         * @return Name of branch.
         */
        [[nodiscard]] string name() const;

        /**
         * Gets the branch reference name.
         *
         * @return Reference name.
         */
        [[nodiscard]] string reference_name() const;

        /**
         * Determine if HEAD points to the given branch
         *
         * @return True if branch is currently the HEAD.
         */
        [[nodiscard]] bool is_head() const;

        /**
         * Get the OID pointed to by a direct reference.
         * Only available if the reference is direct (i.e. an object id reference, not a symbolic one).
         *
         * To find the OID of a symbolic ref, call `git_reference_resolve()` and then this function (or maybe use
         * `git_reference_name_to_id()` to directly resolve a reference name all the way through to an OID).
         *
         * @return Branch target OID.
         */
        [[nodiscard]] OID target() const;

        /**
         * Conditionally create a new reference with the same name as the given reference but a different OID target.
         * The reference must be a direct reference, otherwise this will fail.
         *
         * The new reference will be written to disk, overwriting the given reference.
         *
         * @param oid OID to set target to.
         * @param log_message Log message to use when setting target.
         */
        void set_target(OID, const char *);

        /**
         * Delete an existing reference.
         *
         * This method works for both direct and symbolic references.
         *
         * This function will return an error if the reference has changed from the time it was looked up.
         */
        void delete_reference() const;


        /**
         * Delete an existing branch.
         */
        void delete_branch() const;
    };
}