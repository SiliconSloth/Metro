namespace metro {
    string default_merge_message(const string& mergedName);

    string get_merge_message(const Repository& repo);
    void set_merge_message(const Repository& repo, const string& message);

    // Get the commit ID of the merge head. Assumes a merge is ongoing.
    string merge_head_id(const Repository& repo);

    void start_merge(const Repository& repo, const string& sourceName);

    // Create a commit of the ongoing merge and clear the merge state and conflicts from the repo.
    void resolve(const Repository& repo);

    bool absorb(const Repository& repo, const string& mergeHead);
}
