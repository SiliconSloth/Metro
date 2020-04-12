namespace metro {
    string default_merge_message(const string& mergedName) {
        return "Absorbed " + mergedName;
    }

    string get_merge_message(const Repository& repo) {
        return read_all(repo.path() + "/MERGE_MSG");
    }

    void set_merge_message(const Repository& repo, const string& message) {
        write_all(message, repo.path() + "/MERGE_MSG");
    }

    string merge_head_id(const Repository& repo) {
        return get_commit(repo, "MERGE_HEAD").id().str();
    }

    void start_merge(const Repository& repo, const string& name) {
        Commit otherHead = get_commit(repo, name);
        AnnotatedCommit annotatedOther = repo.lookup_annotated_commit(otherHead.id());
        vector<AnnotatedCommit> sources = {annotatedOther};

        git_merge_analysis_t analysis = repo.merge_analysis(sources);
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

        set_merge_message(repo, default_merge_message(name));
    }

    void resolve(const Repository& repo) {
        if (!merge_ongoing(repo)) {
            throw NotMergingException();
        }

        // Get the merge details before the merge state is cleared.
        string mergeHead = merge_head_id(repo);
        string message = get_merge_message(repo);

        repo.cleanup_state();
        repo.index().cleanup_conflicts();
        commit(repo, message, {"HEAD", mergeHead});
    }

    bool absorb(const Repository& repo, const string& mergeHead) {
        if (is_wip(mergeHead)) {
            throw UnsupportedOperationException("Can't absorb WIP branch.");
        }
        assert_not_merging(repo);

        start_merge(repo, mergeHead);
        if (repo.index().has_conflicts()) {
            return true;
        } else {
            // If no conflicts occurred make the merge commit right away.
            resolve(repo);
            return false;
        }
    }
}