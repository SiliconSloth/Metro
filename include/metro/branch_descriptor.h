/*
 * This file contains code regarding the format of branch names.
 */

#pragma once

#define WIP_SUFFIX "#wip"
// Account for termination character.
#define WIP_SUFFIX_LENGTH (sizeof(WIP_SUFFIX) - 1)

namespace metro {
    // A useful way to manipulate the separate parts of a branch name.
    struct BranchDescriptor {
        string baseName;
        int version = 0;
        bool wip = false;

        // Convert a branch name into a descriptor.
        explicit BranchDescriptor(string name);

        /**
         * Convert the descriptor to a correctly formatted branch name.
         * @return Full name of branch.
         */
        [[nodiscard]] string full_name() const;
    };

    /**
     * Append the WIP suffix to a branch name, if not already present.
     * @param name Name of branch to add WIP to.
     * @return Name of branch with WIP suffix added.
     */
    string to_wip(const string& name);

    /**
     * Remove the WIP suffix from a branch name, if present.
     * @param name Name of branch to remove WIP from.
     * @return Name of branch with WIP suffix removed.
     */
    string un_wip(const string& name);

    /**
     * Check if a branch name has the WIP suffix.
     * @param name Name of branch to test WIP suffix.
     * @return True if the branch has a WIP suffix.
     */
    bool is_wip(const string& name);
}
