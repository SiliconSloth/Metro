namespace metro {
    enum SyncDirection {UP, DOWN, BOTH};

    Repository clone(const string& url, const string& path);
    Repository clone(const string& url, const string& path, CredentialStore *credentials);
    void sync(const Repository& repo, SyncDirection direction, bool force);
    void sync(const Repository& repo, CredentialStore *credentials, SyncDirection direction, bool force);
    void force_pull(const Repository& repo);
}