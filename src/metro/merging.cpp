namespace metro {
    string default_merge_message(const string& mergedName) {
        return "Merge commit '" + mergedName + "'";
    }

    string get_merge_message(const Repository& repo) {
        return read_all(repo.path() + "/MERGE_MSG");
    }

    void set_merge_message(const Repository& repo, const string& message) {
        write_all(message, repo.path() + "/MERGE_MSG");
    }

    void start_merge(const Repository& repo, const string& name) {
        Commit otherHead = get_commit(repo, name);
        AnnotatedCommit annotatedOther = repo.lookup_annotated_commit(otherHead.id());
        vector<AnnotatedCommit> sources = {annotatedOther};

        git_merge_analysis_t analysis = repo.merge_analysis(sources);
        // TODO: Possible bug: What if a WIP being restored is a merge of two branches that are up-to-date?
        if ((analysis & (GIT_MERGE_ANALYSIS_NONE | GIT_MERGE_ANALYSIS_UP_TO_DATE)) != 0) {
            throw UnnecessaryMergeException();
        }
        if ((analysis & GIT_MERGE_ANALYSIS_NORMAL) == 0) {
            throw UnsupportedOperationException("Non-normal absorb not supported.");
        }

        git_merge_options mergeOpts = GIT_MERGE_OPTIONS_INIT;
        git_checkout_options checkoutOpts = GIT_CHECKOUT_OPTIONS_INIT;
        checkoutOpts.checkout_strategy = GIT_CHECKOUT_FORCE | GIT_CHECKOUT_ALLOW_CONFLICTS;
        repo.merge(sources, mergeOpts, checkoutOpts);
    }
}