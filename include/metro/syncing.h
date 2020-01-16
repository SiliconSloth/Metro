namespace metro {
    string name_from_url(const string& url);
    Repository clone(const string& url, const string& path);
    Repository clone(const string& url, const string& path, git_cred** credentials);
    void sync(const Repository& repo);
    void sync(const Repository& repo, git_cred** credentials);
}