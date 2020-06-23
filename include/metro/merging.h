/*
 * Code regarding merging branches.
 */

#pragma once

namespace metro {
    /**
     * The commit message Metro uses when absorbing a commit referenced by the given name.
     * @param mergedName Branch being merged.
     * @return The default message for that merge.
     */
    string default_merge_message(const string& mergedName);

    /**
     * Gets the stored merge message from the repo.
     * @param repo Repo to find the merge message from.
     * @return The message for the current merge.
     */
    string get_merge_message(const Repository& repo);

    /**
     * Sets the stored merge message from the repo.
     * @param repo Repo to store the merge message in.
     * @param message The message to store for the current merge.
     */
    void set_merge_message(const Repository& repo, const string& message);

    /**
     * Merge the specified commit into the current branch head.
     * The repo will be left in a merging state, possibly with conflicts in the index.
     * @param repo Repo to begin the merge on.
     * @param sourceName The name of the source commit to merge in.
     */
    void start_merge(const Repository& repo, const string& sourceName);
}
