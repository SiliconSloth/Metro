#include "pch.h"

namespace metro {
    // The commit message Metro uses when absorbing a commit referenced by the given name.
    string default_merge_message(const string& mergedName) {
        return "Absorbed " + mergedName;
    }

    string get_merge_message(const Repository& repo) {
        return read_all(repo.path() + "/MERGE_MSG");
    }

    void set_merge_message(const Repository& repo, const string& message) {
        write_all(message, repo.path() + "/MERGE_MSG");
    }

    // Merge the specified commit into the current branch head.
    // The repo will be left in a merging state, possibly with conflicts in the index.
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
}