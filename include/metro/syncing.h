namespace metro {
    Repository clone(const string& url, const string& path);
    Repository clone(const string& url, const string& path, CredentialStore *credentials);
    void sync(const Repository& repo);
    void sync(const Repository& repo, CredentialStore *credentials);
}