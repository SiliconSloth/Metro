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

        // Set the base or WIP target according to the above rules.
        void add_target(const OID& target, bool wip);
    };

    struct RefTargets {
        DualTarget local;
        DualTarget remote;
        DualTarget synced;
    };

    enum SyncType {PUSH, PULL, CONFLICT};
    enum SyncDirection {UP, DOWN, BOTH};

    Repository clone(const string& url, const string& path);
    Repository clone(const string& url, const string& path, CredentialStore *credentials);
    void sync(const Repository& repo, SyncDirection direction, bool force);
    void sync(const Repository& repo, CredentialStore *credentials, SyncDirection direction, bool force);
    void force_pull(const Repository& repo);
}