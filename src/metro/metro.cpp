#include "pch.h"

using namespace git;

namespace metro {
    // Tests if the repo is currently merging.
    // repo: The repo
    // returns: true if the repo is currently in merging state
    bool merge_ongoing(const Repository& repo) {
        try {
            // Revision will only exist if merging
            repo.revparse_single("MERGE_HEAD");
        } catch (GitException&) {
            return false;
        }
        return true;
    }

    // Asserts that there is an ongoing merge
    // repo: The repo
    // throw: CurrentlyMergingException if merging
    void assert_merging(const Repository& repo) {
        if (merge_ongoing(repo)) {
            throw CurrentlyMergingException();
        }
    }

    // Finds differences between head and working dir index (must git add first)
    // repo: The repo
    // return: The diff created between head and working dir
    Diff current_changes(const Repository& repo) {
        Tree current = get_commit(repo, "HEAD").tree();
        DiffOptions opts = GIT_DIFF_OPTIONS_INIT;
        Diff diff = Diff::tree_to_workdir_with_index(repo, current, &opts);

        return diff;
    }

    // Commit all files in the repo directory (excluding those in .gitignore) to updateRef.
    // repo: The repo
    // updateRef: The reference to update to point to the new commit, e.g. "HEAD" to commit to the head of the current branch
    // message: The commit message
    // parentCommits: The commit's parents
    void commit(const Repository& repo, const string& updateRef, const string& message, const vector<Commit>& parentCommits) {
        Signature author = repo.default_signature();

        Index index = add_all(repo);
        // Write the files in the index into a tree that can be attached to the commit.
        OID oid = index.write_tree();
        Tree tree = repo.lookup_tree(oid);
        // Save the index to disk so that it stays in sync with the contents of the working directory.
        // If we don't do this removals of every file are left staged.
        index.write();

        // Commit the files to the head of the current branch.
        repo.create_commit(updateRef, author, author, "UTF-8", message, tree, parentCommits);
    }

    // Commit all files in the repo directory (excluding those in .gitignore) to updateRef.
    // updateRef: The reference to update to point to the new commit, e.g. "HEAD" to commit to the head of the current branch
    // repo: The repo
    // message: The commit message
    // parentRevs: The revisions corresponding to the commit's parents
    void commit(const Repository& repo, const string& updateRef, const string& message, const initializer_list<string> parentRevs) {
        // Retrieve the commit objects associated with the given parent revisions.
        vector<Commit> parentCommits;
        for (const string& parentRev : parentRevs) {
            parentCommits.push_back(static_cast<Commit>(repo.revparse_single(parentRev)));
        }

        commit(repo, updateRef, message, parentCommits);
    }

    // Commit all files in the repo directory (excluding those in .gitignore) to the head of the current branch.
    // repo: The repo
    // message: The commit message
    // parentCommits: The commit's parents
    void commit(const Repository& repo, const string& message, const vector<Commit>& parentCommits) {
        commit(repo, "HEAD", message, parentCommits);
    }

    // Commit all files in the repo directory (excluding those in .gitignore) to the head of the current branch.
    // repo: The repo
    // message: The commit message
    // parentRevs: The revisions corresponding to the commit's parents
    void commit(const Repository& repo, const string& message, initializer_list<string> parentRevs) {
        commit(repo, "HEAD", message, parentRevs);
    }

    Index add_all(const Repository& repo) {
        Index index = repo.index();
        index.add_all(StrArray(), GIT_INDEX_ADD_DISABLE_PATHSPEC_MATCH, nullptr);
        return index;
    }

    // Initialize an empty git repository in the specified directory,
    // with an initial commit.
    Repository create(const string& path) {
        if (Repository::exists(path+"/.git")) {
            throw RepositoryExistsException();
        }

        Repository repo = Repository::init(path+"/.git", false);
        commit(repo, "Create repository", {});
        return repo;
    }

    void delete_last_commit(const Repository& repo, bool reset) {
        Commit lastCommit = get_commit(repo, "HEAD");
        if (lastCommit.parentcount() == 0) {
            throw UnsupportedOperationException("Can't delete initial commit.");
        }
        Commit parent = lastCommit.parent(0);

        git_checkout_options checkoutOpts = GIT_CHECKOUT_OPTIONS_INIT;
        checkoutOpts.checkout_strategy = GIT_CHECKOUT_FORCE;
        git_reset_t resetType = reset? GIT_RESET_HARD : GIT_RESET_SOFT;

        repo.reset_to_commit(parent, resetType, checkoutOpts);
    }

    void patch(const Repository& repo, const string& message) {
        assert_merging(repo);
        vector<Commit> parents = get_commit(repo, "HEAD").parents();
        delete_last_commit(repo, false);
        commit(repo, message, parents);
    }

    // Gets the commit corresponding to the given revision
    // repo - Repo to find the commit in
    // revision - Revision of the commit to find
    //
    // Returns the commit
    Commit get_commit(const Repository& repo, const string& revision) {
        Object object = repo.revparse_single(revision);
        Commit commit = (Commit) object;
        return commit;
    }

    bool commit_exists(const Repository &repo, const string& name) {
        try {
            get_commit(repo, name);
            return true;
        } catch (GitException&) {
            return false;
        }
    }

    // Create a new branch from the current head with the specified name.
    // Returns the branch
    void create_branch(const Repository &repo, const string& name) {
        Commit commit = get_commit(repo, "HEAD");
        repo.create_branch(name, commit, false);
    }

    bool branch_exists(const Repository &repo, const string& name) {
        try {
            repo.lookup_branch(name, GIT_BRANCH_LOCAL);
            return true;
        } catch (GitException&) {
            return false;
        }
    }

    int transfer_progress_callback(const git_transfer_progress *stats, void *) {
        int progress = 100 * (stats->received_objects + stats->indexed_objects) / (stats->total_objects);
        printf("\rProgress: %d%%", progress);
        if (progress == 100) {
            cout << endl;
        } else if (progress > 100) {
            cout << "Progress Tracking Error: Please send debug report to developers" << endl;
            printf("Recieved Objects: %d, Indexed Objects: %d, Total Objects: %d, Total Progress:%d\n",
                    stats->received_objects, stats->indexed_objects, stats->total_objects, progress);
        }
        return GIT_OK;
    }

    string current_branch_name(const Repository& repo) {
        BranchIterator iter = repo.new_branch_iterator(GIT_BRANCH_LOCAL);
        for (Branch branch; iter.next(&branch);) {
            if (branch.is_head()) {
                return branch.name();
            }
        }
        throw BranchNotFoundException();
    }

    void delete_branch(const Repository& repo, const string& name) {
        // If the user tries to delete the current branch,
        // we must switch out of it first.
        // Preferably switch into the master branch,
        // but if that does not exist just pick an arbitrary branch.
        if (name == current_branch_name(repo)) {
            if (branch_exists(repo, "master") && name != "master") {
                switch_branch(repo, "master");
            } else {
                bool found = false;
                BranchIterator iter = repo.new_branch_iterator(GIT_BRANCH_LOCAL);
                for (Branch branch; iter.next(&branch);) {
                    // Pick any branch that isn't the one being deleted and isn't a WIP branch.
                    if (branch.name() != name && !is_wip(branch.name())) {
                        switch_branch(repo, branch.name());
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    throw UnsupportedOperationException("Can't delete only non-WIP branch");
                }
            }
        }

        Branch branch = repo.lookup_branch(name, GIT_BRANCH_LOCAL);
        branch.delete_branch();

        if (branch_exists(repo, to_wip(name))) {
            Branch branch = repo.lookup_branch(to_wip(name), GIT_BRANCH_LOCAL);
            branch.delete_branch();
        }
    }

    // Checks out the given commit without moving head,
    // such that the working directory will match the commit contents.
    // Doesn't change current branch ref.
    void checkout(const Repository& repo, const string& name) {
        Tree tree = get_commit(repo, name).tree();
        git_checkout_options checkoutOpts = GIT_CHECKOUT_OPTIONS_INIT;
        checkoutOpts.checkout_strategy = GIT_CHECKOUT_FORCE;
        repo.checkout_tree(tree, checkoutOpts);
    }

    bool has_uncommitted_changes(const Repository& repo) {
        git_status_options opts = GIT_STATUS_OPTIONS_INIT;
        opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
        opts.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED;

        StatusList status = repo.new_status_list(opts);
        return status.entrycount() > 0;
    }

    vector<StandaloneConflict> get_conflicts(const Index& index) {
        vector<StandaloneConflict> conflicts;
        ConflictIterator iter = index.conflict_iterator();
        for (Conflict conflict{}; iter.next(conflict);) {
            conflicts.emplace_back(StandaloneConflict(conflict));
        }
        return conflicts;
    }

    // If the working directory has changes since the last commit, or a merge has been started,
    // Save these changes in a WIP commit in a new #wip branch.
    void save_wip(const Repository& repo) {
        // If there are no changes since the last commit, don't bother with a WIP commit.
        if (!(has_uncommitted_changes(repo) || merge_ongoing(repo))) {
            return;
        }

        string name = current_branch_name(repo);
        string wipName = to_wip(name);

        try {
            delete_branch(repo, wipName);
        } catch (GitException&) {
            // We don't mind if the delete fails, we tried it just in case.
        }
        create_branch(repo, wipName);

        if (merge_ongoing(repo)) {
            // Store the merge message in the second line (and beyond) of the WIP commit message.
            string message = get_merge_message(repo);
            commit(repo, "refs/heads/"+wipName, "WIP\n"+message, {"HEAD", "MERGE_HEAD"});
            repo.cleanup_state();
        } else {
            commit(repo, "refs/heads/"+wipName, "WIP", {"HEAD"});
        }
    }

    // Deletes the WIP commit at head if any, restoring the contents to the working directory
    // and resuming a merge if one was ongoing.
    void restore_wip(const Repository& repo) {
        string name = current_branch_name(repo);
        string wipName = to_wip(name);

        if (!branch_exists(repo, wipName)) {
            return;
        }
        Commit wipCommit = get_commit(repo, wipName);
        Index index = repo.index();

        vector<StandaloneConflict> conflicts;
        // If the WIP commit has two parents a merge was ongoing.
        if (wipCommit.parentcount() > 1) {
            string mergeHead = wipCommit.parent(1).id().str();
            start_merge(repo, mergeHead);

            // Reload the merge message from before, stored in the second line (and beyond)
            // of the WIP commit message.
            string commitMessage = wipCommit.message();
            size_t newlineIndex = commitMessage.find('\n');
            // If the commit message only has one line (only happens if it has been tampered with)
            // just leave the message as the default one created when restarting the merge.
            // Otherwise restore the merge message from the commit message.
            if (newlineIndex >= 0) {
                string mergeMessage = commitMessage.substr(newlineIndex+1, string::npos);
                set_merge_message(repo, mergeMessage);
            }

            // Remove the conflicts from the index temporarily so we can checkout.
            // They will be restored after so that the index and working dir
            // match their state when the WIP commit was created.
            conflicts = get_conflicts(index);
            index.cleanup_conflicts();
        }

        // Restore the contents of the WIP commit to the working directory.
        checkout(repo, wipName);
        delete_branch(repo, wipName);

        // If we are mid-merge, restore the conflicts from the merge.
        for (const Conflict& conflict : conflicts) {
            index.add_conflict(conflict);
        }
        index.write();
    }

    void switch_branch(const Repository& repo, const string& name) {
        if (is_wip(name)) {
            throw UnsupportedOperationException("Can't switch to WIP branch.");
        }
        if (!branch_exists(repo, name)) {
            throw BranchNotFoundException();
        }

        save_wip(repo);
        checkout(repo, name);
        move_head(repo, name);
        restore_wip(repo);
    }

    void move_head(const Repository& repo, const string& name) {
        Branch branch = repo.lookup_branch(name, GIT_BRANCH_LOCAL);
        repo.set_head(branch.reference_name());
    }

    Commit get_last_commit(const Repository &repo) {
        return get_commit(repo, "HEAD");
    }

    void reset_head(const Repository &repo) {
        Commit head = get_last_commit(repo);
        repo.reset_to_commit(head, GIT_RESET_HARD, git_checkout_options{});
    }

    // Replaces all current work with new branch, resetting the commit
    // Does NOT check if safe - do that first
    void fast_forward(const Repository &repo, string name) {
        // Replaces all current work with origin
        string branch = current_branch_name(repo);
        checkout(repo, name);
        Branch ref = repo.lookup_branch(branch, GIT_BRANCH_LOCAL);
        Branch refOrigin = repo.lookup_branch("origin/" + branch, GIT_BRANCH_REMOTE);
        ref.set_target(refOrigin.target(), "message");
        checkout_branch(repo, branch);
    }

    Remote add_remote(const Repository &repo, string url) {
        StrArray remotes = repo.remote_list();

        if (remotes.count() < 1) {
            Remote remote = repo.remote_create("origin", url);
            return remote;
        } else {
            repo.remote_set_url("origin", url);
            return repo.lookup_remote(remotes.strings()[0]);
        }
    }

    MergeAnalysis merge_analysis(const Repository &repo, string name) {
        Commit otherHead = get_commit(repo, name);
        AnnotatedCommit annOther = repo.lookup_annotated_commit(otherHead.id());
        vector<AnnotatedCommit> sources;
        sources.push_back(annOther);

        return repo.merge_analysis(sources);
    }

    // Checks out the given branch by name
    // name - Plain Text branch name (e.g. 'master')
    // repo - Repo to Checkout from
    void checkout_branch(Repository repo, string name) {
        checkout(repo, name);
        move_head(repo, name);
    }

    StrArray reference_list(const Repository& repo) {
        git_strarray refs;
        int error = git_reference_list(&refs, repo.ptr().get());
        return StrArray(&refs);
    }

    string time_to_string(Time time) {
        char buf[80];
        struct tm ts = *localtime(&time.time);
        strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Y ", &ts);

        int hour_offset = 0;
        int minute_offset = time.offset;
        while (minute_offset >= 60) {
            minute_offset -= 60;
            hour_offset++;
        }

        char buf2[5];
        sprintf(buf2, "%02d%02d", hour_offset, minute_offset);

        char buf3[100];
        sprintf(buf3, "%s%c%s", buf, time.sign, buf2);

        return string(buf3);
    }

    // Should be in format like "rgbi----" or "r--i-gb-"
    // rgb is colour, i is intensity. The first 4 are the
    // text, and the second 4 are the background
    void set_text_colour(string colour, void* handle) {
#ifdef _WIN32
        int current = 0;
        if (colour[0] == 'r') current |= FOREGROUND_RED;
        if (colour[1] == 'g') current |= FOREGROUND_GREEN;
        if (colour[2] == 'b') current |= FOREGROUND_BLUE;
        if (colour[3] == 'i') current |= FOREGROUND_INTENSITY;
        if (colour[4] == 'r') current |= BACKGROUND_RED;
        if (colour[5] == 'g') current |= BACKGROUND_GREEN;
        if (colour[6] == 'b') current |= BACKGROUND_BLUE;
        if (colour[7] == 'i') current |= BACKGROUND_INTENSITY;

        SetConsoleTextAttribute(handle, current);
#endif //_WIN32
    }
}