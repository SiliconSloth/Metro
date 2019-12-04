namespace metro {
    string name_from_url(const string& url);
    Repository clone(const string& url, const string& path);
    void sync(const Repository& repo);
    void sync_down(const Repository& repo, bool force);
}