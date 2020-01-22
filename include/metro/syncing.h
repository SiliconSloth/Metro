namespace metro {
    Repository clone(const string& url, const string& path);
    Repository clone(const string& url, const string& path, git_cred** credentials);
    void sync(const Repository& repo);
    void sync(const Repository& repo, git_cred** credentials);
}