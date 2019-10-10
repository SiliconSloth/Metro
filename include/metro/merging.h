namespace metro {
    string default_merge_message(const string& mergedName);

    string get_merge_message(const Repository& repo);
    void set_merge_message(const Repository& repo, const string& message);

    void start_merge(const Repository& repo, const string& sourceName);
}
