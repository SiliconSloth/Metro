/*
 * Code relating to syncing functionality
 */

#pragma once

namespace metro {
    // Collection of targets
    struct RefTargets {
        OID local;
        OID remote;
        OID synced;
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