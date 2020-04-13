/*
 * Code relating to syncing functionality
 */

#pragma once

namespace metro {
    /*
     * Represents a base branch and it's corresponding WIP branch.
     * base is the target of the base branch. If there is no WIP branch for that base branch, head == base.
     * If there is a WIP branch, then head is the target of the WIP branch.
     */
    struct DualTarget {
        OID head;
        OID base;
        bool hasWip = false;

        /**
         * Set the base or WIP target according to the above rules.
         *
         * @param target Target to set the base to.
         * @param wip True to set WIP target instead.
         */
        void add_target(const OID& target, bool wip);

        /**
         * Finds whether the target is valid for sync.
         * 
         * @returns True if this DualTarget has no WIP branch or base is the first parent of head.
         * If this is not the case, then the dual branch is invalid and cannot be correctly synced.
         */
        bool is_valid(const Repository& repo) const;
    };

    // Collection of targets
    struct RefTargets {
        DualTarget local;
        DualTarget remote;
        DualTarget synced;
    };

    /**
     * Enum representing types of sync that can be chosen to occur.
     */
    enum SyncType {PUSH, PULL, CONFLICT};

    /**
     * Enum representing the direction of syncing chosen by the user.
     */
    enum SyncDirection {UP, DOWN, BOTH};

    /**
     * Clones a repo from the given url to the given path.
     * @param url The url to clone.
     * @param path The path to clone to.
     * @return The cloned repository.
     */
    Repository clone(const string& url, const string& path);

    /**
    * Clones a repo from the given url to the given path.
    * @param url The url to clone.
    * @param path The path to clone to.
    * @param credentials The credentials used for cloning.
    * @return The cloned repository.
    */
    Repository clone(const string& url, const string& path, CredentialStore *credentials);

    /**
     * Syncs the repo with the remote version.
     * @param repo The repo to sync.
     * @param direction The direction that can be synced.
     * @param force Whether to force sync.
     */
    void sync(const Repository& repo, SyncDirection direction, bool force);

    /**
     * Syncs the repo with the remote version.
     * @param repo The repo to sync.
     * @param credentials The credentials used for syncing.
     * @param direction The direction that can be synced.
     * @param force Whether to force sync.
     */
    void sync(const Repository& repo, CredentialStore *credentials, SyncDirection direction, bool force);

    /**
     * Pulls all the repo branches assuming the remote is correct
     * @param repo The repo to force pull within.
     */
    void force_pull(const Repository& repo);
}
