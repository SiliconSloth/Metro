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
     * Get the commit ID of the merge head. Assumes a merge is ongoing.
     * @param repo Repo to find merge head for ID.
     * @return The commit id of the merge head.
     */
    string merge_head_id(const Repository& repo);

    /**
     * Merge the specified commit into the current branch head.
     * The repo will be left in a merging state, possibly with conflicts in the index.
     * @param repo Repo to begin the merge on.
     * @param sourceName The name of the source commit to merge in.
     */
    void start_merge(const Repository& repo, const string& sourceName);

    /**
     * Create a commit of the ongoing merge and clear the merge state and conflicts from the repo.
     * @param repo Repo to resolve the merge on.
     */
    void resolve(const Repository& repo);

    /**
     * Absorbs the target branch into the current branch.
     * @param repo Repository to make merge in
     * @param mergeHead The commit to merge into current.
     * @return True if conflicts occurred during merge.
     */
    bool absorb(const Repository& repo, const string& mergeHead);
}
