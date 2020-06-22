namespace metro {

    bool merge_ongoing(const Repository &repo) {
        try {
            // Revision will only exist if merging
            repo.revparse_single("MERGE_HEAD");
        } catch (GitException &) {
            return false;
        }
        return true;
    }

    void assert_not_merging(const Repository &repo) {
        if (merge_ongoing(repo)) {
            throw CurrentlyMergingException();
        }
    }

    Tree working_tree(const Repository &repo) {
        Index index = add_all(repo);
        // Write the files in the index into a tree that can be attached to the commit.
        OID oid = index.write_tree();
        Tree tree = repo.lookup_tree(oid);
        // Save the index to disk so that it stays in sync with the contents of the working directory.
        // If we don't do this removals of every file are left staged.
        index.write();

        return tree;
    }

    Diff current_changes(const Repository &repo) {
        Tree current = Tree();
        try {
            current = get_commit(repo, "HEAD").tree();
        } catch (GitException &ex) {
            // The current branch might have no commits, which is ok.
        }
        git_diff_options opts = GIT_DIFF_OPTIONS_INIT;
        Diff diff = Diff::tree_to_workdir_with_index(repo, current, &opts);

        return diff;
    }

    void commit(const Repository &repo, const string &updateRef, const string &message,
                const vector<Commit> &parentCommits) {
        git_signature author = repo.default_signature();
        Tree tree = working_tree(repo);

        // Commit the files to the head of the current branch.
        repo.create_commit(updateRef, author, author, "UTF-8", message, tree, parentCommits);
    }

    void commit(const Repository &repo, const string &updateRef, const string &message,
                const initializer_list<string> parentRevs) {
        // Retrieve the commit objects associated with the given parent revisions.
        vector<Commit> parentCommits;
        for (const string &parentRev : parentRevs) {
            parentCommits.push_back(static_cast<Commit>(repo.revparse_single(parentRev)));
        }

        commit(repo, updateRef, message, parentCommits);
    }

    void commit(const Repository &repo, const string &message, const vector<Commit> &parentCommits) {
        commit(repo, "HEAD", message, parentCommits);
    }

    void commit(const Repository &repo, const string &message, initializer_list<string> parentRevs) {
        commit(repo, "HEAD", message, parentRevs);
    }

    Index add_all(const Repository &repo) {
        Index index = repo.index();
        index.add_all(StrArray(), GIT_INDEX_ADD_DISABLE_PATHSPEC_MATCH, nullptr);
        return index;
    }

    Repository create(const string &path) {
        if (Repository::exists(path + "/.git")) {
            throw RepositoryExistsException();
        }

        try {
            Repository repo = Repository::init(path + "/.git", false);
            commit(repo, "Create repository", {});
            return repo;
        } catch (GitException &e) {
            string error(e.what());
            int pos = error.find("The filename, directory name, or volume label syntax is incorrect.");
            if (pos >= 0 && pos <= error.length()) {
                throw MetroException(
                        "Unable to create directory '" + path + "' as that name is disallowed on this OS.");
            }
            throw e;
        }
    }

    void delete_last_commit(const Repository &repo, bool reset) {
        if (!head_exists(repo)) {
            throw MetroException("No commit to delete.");
        }

        Commit lastCommit = get_commit(repo, "HEAD");
        if (lastCommit.parentcount() == 0) {
            throw UnsupportedOperationException("Can't delete initial commit.");
        }
        Commit parent = lastCommit.parent(0);
        reset_head(repo, parent, reset);
    }

    void patch(const Repository &repo, const string &message) {
        assert_not_merging(repo);

        git_signature author = repo.default_signature();
        Tree tree = working_tree(repo);
        Commit commit = get_commit(repo, "HEAD");

        commit.amend("HEAD", author, author, "UTF-8", message, tree);
    }

    Commit get_commit(const Repository &repo, const string &revision) {
        Object object = repo.revparse_single(revision);
        Commit commit = (Commit) object;
        return commit;
    }

    bool commit_exists(const Repository &repo, const string &name) {
        try {
            get_commit(repo, name);
            return true;
        } catch (GitException &) {
            return false;
        }
    }

    void create_branch(const Repository &repo, const string &name) {
        Commit commit = get_commit(repo, "HEAD");
        repo.create_branch(name, commit, false);
    }

    bool branch_exists(const Repository &repo, const string &name) {
        try {
            repo.lookup_branch(name, GIT_BRANCH_LOCAL);
            return true;
        } catch (GitException &) {
            return false;
        }
    }

    Head get_head(const Repository &repo) {
        string name = read_all(repo.path() + "HEAD");
        if (has_prefix(name, "ref: refs/")) {
            name = name.substr(10, string::npos);
            if (has_prefix(name, "heads/")) {
                name = name.substr(6, string::npos);
            } else if (has_prefix(name, "remotes/")) {
                name = name.substr(8, string::npos);
            }
        }

        if (has_suffix(name, "\n")) {
            name = name.substr(0, name.size() - 1);
        }

        return Head{name, repo.head_detached()};
    }

    bool is_on_branch(const Repository &repo, const string &branch) {
        const Head head = get_head(repo);
        return !head.detached && head.name == branch;
    }

    bool head_exists(const Repository &repo) {
        return commit_exists(repo, "HEAD");
    }

    void delete_branch(const Repository &repo, const string &name) {
        // If the user tries to delete the current branch,
        // we must switch out of it first.
        // Preferably switch into the master branch,
        // but if that does not exist just pick an arbitrary branch.
        if (is_on_branch(repo, name)) {
            if (branch_exists(repo, "master") && name != "master") {
                // Don't try to restore after switching if the branch being deleted is the #wip branch.
                switch_branch(repo, "master", false, name != to_wip("master"));
            } else {
                bool found = false;
                BranchIterator iter = repo.new_branch_iterator(GIT_BRANCH_LOCAL);
                for (Branch branch; iter.next(&branch);) {
                    // Pick any branch that isn't the one being deleted and isn't a WIP branch.
                    if (branch.name() != name && !is_wip(branch.name())) {
                        // Don't try to restore after switching if the branch being deleted is the #wip branch.
                        switch_branch(repo, branch.name(), false, name != to_wip(branch.name()));
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    throw UnsupportedOperationException("Can't delete only non-WIP branch");
                }
            }
        }

        if (!branch_exists(repo, name)) throw BranchNotFoundException(name);

        Branch branch = repo.lookup_branch(name, GIT_BRANCH_LOCAL);
        branch.delete_branch();

        // Also delete the WIP branch if present.
        if (branch_exists(repo, to_wip(name))) {
            Branch branch = repo.lookup_branch(to_wip(name), GIT_BRANCH_LOCAL);
            branch.delete_branch();
        }
    }

    void checkout(const Repository &repo, const string &name) {
        checkout(repo, get_commit(repo, name));
    }

    void checkout(const Repository &repo, const Commit &commit) {
        Tree tree = commit.tree();
        git_checkout_options checkoutOpts = GIT_CHECKOUT_OPTIONS_INIT;
        checkoutOpts.checkout_strategy = GIT_CHECKOUT_FORCE;
        repo.checkout_tree(tree, checkoutOpts);
    }

    bool has_uncommitted_changes(const Repository &repo) {
        git_status_options opts = GIT_STATUS_OPTIONS_INIT;
        opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
        opts.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED;

        StatusList status = repo.new_status_list(opts);
        return status.entrycount() > 0;
    }

    vector<StandaloneConflict> get_conflicts(const Index &index) {
        vector<StandaloneConflict> conflicts;
        ConflictIterator iter = index.conflict_iterator();
        for (Conflict conflict{}; iter.next(conflict);) {
            conflicts.emplace_back(StandaloneConflict(conflict));
        }
        return conflicts;
    }

    void save_wip(const Repository &repo) {
        // If there are no changes since the last commit, don't bother with a WIP commit.
        if (!(has_uncommitted_changes(repo) || merge_ongoing(repo))) {
            return;
        }

        Head head = get_head(repo);
        bool headExists = head_exists(repo);
        if (!headExists) head = metro::Head(string("master"), false);
        if (head.detached) {
            throw UnsupportedOperationException("Attempted to save WIP with detached head");
        }
        string wipName = to_wip(head.name);

        if (branch_exists(repo, wipName)) {
            throw UnexpectedWIPException(wipName);
        }
        if (merge_ongoing(repo)) {
            // Store the merge message in the second line (and beyond) of the WIP commit message.
            string message = get_merge_message(repo);
            if (headExists) {
                commit(repo, "refs/heads/" + wipName, "WIP\n" + message, {"HEAD", "MERGE_HEAD"});
            } else {
                commit(repo, "refs/heads/" + wipName, "WIP\n" + message, {"MERGE_HEAD"});
            }
            repo.cleanup_state();
        } else {
            if (headExists) {
                commit(repo, "refs/heads/" + wipName, "WIP", {"HEAD"});
            } else {
                commit(repo, "refs/heads/" + wipName, "WIP", {});
            }
        }

        if (headExists) {
            reset_head(repo, get_commit(repo, get_head(repo).name), true);
        } else {
            reset_to_empty(repo);
        }
    }

    void restore_wip(const Repository& repo, bool force) {
        // Ensure working dir is empty
        add_all(repo);
        if (!force && current_changes(repo).num_deltas() != 0) {
            throw MetroException("Couldn't restore WIP because the working directory has changed.\n"
                                 "To replace the working directory, you can use 'metro wip restore --force'");
        }

        // Ensure head is attached
        Head head = get_head(repo);
        if (head.detached) {
            throw UnsupportedOperationException("Attempted to restore WIP with detached head");
        }
        string wipName = to_wip(head.name);

        if (!branch_exists(repo, wipName)) {
            return;
        }

        // Ensure the WIP is valid
        if (head_exists(repo)) {
            OID oid = repo.lookup_branch(head.name, GIT_BRANCH_LOCAL).target();
            OID wip_oid = repo.lookup_branch(to_wip(head.name), GIT_BRANCH_LOCAL).target();
            Commit target = repo.lookup_commit(wip_oid);
            bool found = false;
            for (Commit parent : target.parents()) {
                if (parent.id() == oid) found = true;
            }
            if (!found) {
                throw InvalidWIPException(head.name, to_wip(head.name));
            }
        } else {
            OID wip_oid = repo.lookup_branch(to_wip(head.name), GIT_BRANCH_LOCAL).target();
            Commit target = repo.lookup_commit(wip_oid);
            if (target.parentcount() != 0) {
                throw InvalidWIPException(head.name, to_wip(head.name));
            }
        }

        // Don't attempt to restore a WIP branch onto itself.
        if (wipName == head.name) {
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

    void squash_wip(const Repository& repo, bool force) {
        // Ensure head is attached
        Head head = get_head(repo);
        if (head.detached) {
            throw UnsupportedOperationException("Attempted to squash WIP with detached head");
        }
        string wipName = to_wip(head.name);

        if (!branch_exists(repo, wipName)) {
            throw NeedWIPException();
        }

        // Ensure the WIP is valid
        if (head_exists(repo)) {
            OID oid = repo.lookup_branch(head.name, GIT_BRANCH_LOCAL).target();
            OID wip_oid = repo.lookup_branch(wipName, GIT_BRANCH_LOCAL).target();
            Commit base = repo.lookup_commit(oid);
            Commit target = repo.lookup_commit(wip_oid);

            Commit pointer = target;
            Commit compare = base;
            vector<Commit> parents;
            parents.push_back(base);
            if (force) {
                compare = repo.lookup_commit(repo.merge_base(oid, wip_oid));
            }
            while (true) {
                if (pointer.id() == compare.id()) break;
                if (pointer.parentcount() == 0) {
                    assert(!force);
                    throw MetroException("There have been commits to master since the WIP was made.\n"
                                         "You can squash anyway using `metro wip squash --force`, but the resulting working\n"
                                         "directory will undo the changes in those commits.\n"
                                         "Alternatively you can move the WIP to a new branch with 'metro rename " +
                                         wipName + " <other>'.\n");
                }
                vector<Commit> ps = pointer.parents();
                for (int i = 1; i < ps.size(); i++) {
                    parents.push_back(ps.at(i));
                }
                pointer = pointer.parent(0);
            }
            add_all(repo);
            Tree current = working_tree(repo);
            checkout(repo, target);
            assert(!parents.empty());
            if (parents.size() == 1) {
                commit(repo, "WIP", parents);
            } else if (parents.size() == 2) {
                commit(repo, default_merge_message(parents.at(1).id().str()), parents);
            } else {
                commit(repo, default_merge_message("Octopus Merge"), parents);
            }
            Commit newCommit = repo.lookup_commit(repo.head().target());
            repo.lookup_branch(head.name, GIT_BRANCH_LOCAL).set_target(newCommit.parent(0).id(), "Squash WIP p1");
            repo.lookup_branch(wipName, GIT_BRANCH_LOCAL).set_target(newCommit.id(), "Squash WIP p2");
            checkout(repo, base);
            git_checkout_options checkoutOpts = GIT_CHECKOUT_OPTIONS_INIT;
            checkoutOpts.checkout_strategy = GIT_CHECKOUT_FORCE;
            repo.checkout_tree(current, checkoutOpts);
        } else {
            OID wip_oid = repo.lookup_branch(wipName, GIT_BRANCH_LOCAL).target();
            Commit target = repo.lookup_commit(wip_oid);

            add_all(repo);
            Tree current = working_tree(repo);
            checkout(repo, target);
            commit(repo, "WIP", {});
            Commit newCommit = repo.lookup_commit(repo.head().target());
            repo.lookup_branch(head.name, GIT_BRANCH_LOCAL).delete_reference();
            repo.lookup_branch(wipName, GIT_BRANCH_LOCAL).set_target(newCommit.id(), "Squash WIP");
            git_checkout_options checkoutOpts = GIT_CHECKOUT_OPTIONS_INIT;
            checkoutOpts.checkout_strategy = GIT_CHECKOUT_FORCE;
            repo.checkout_tree(current, checkoutOpts);
        }
    }

    void switch_branch(const Repository& repo, const string& name, bool saveWip, bool restoreWip) {
        const Commit commit = get_commit(repo, name);

        if (saveWip) {
            save_wip(repo);
        } else {
            reset_head(repo, get_commit(repo, "HEAD"), true);
        }

        checkout(repo, commit);
        move_head(repo, name);

        if (restoreWip && !repo.head_detached()) {
            restore_wip(repo, false);
        }
    }

    void move_head(const Repository& repo, const string& name) {
        if (branch_exists(repo, name)) {
            repo.set_head(repo.lookup_branch(name, GIT_BRANCH_LOCAL).reference_name());
        } else {
            repo.set_head_detached(get_commit(repo, name).id());
        }
    }

    /**
     * Gets the commit at the head of the branch.
     *
     * @param repo Repo to find commit in.
     * @return Commit at head of branch.
     */
    Commit get_last_commit(const Repository &repo) {
        return get_commit(repo, "HEAD");
    }

    /**
     * Sets the url for the origin remote, creating it if it doesn't exist.
     *
     * @param repo Repo to set origin for.
     * @param url Url reference to set as the remote origin.
     * @return Returns the remote created/edited.
     */
    Remote add_remote(const Repository &repo, const string& url) {
        StrArray remotes = repo.remote_list();

        if (remotes.count() < 1) {
            Remote remote = repo.remote_create("origin", url);
            return remote;
        } else {
            repo.remote_set_url("origin", url);
            return repo.lookup_remote(remotes.strings()[0]);
        }
    }

    /**
     * Analyses the given branch against the HEAD for methods of merging.
     *
     * @param repo Repo with branch and HEAD to compare.
     * @param name Name of branch reference to perform merge analysis on.
     * @return The results of analysing the merge.
     */
    git_merge_analysis_t merge_analysis(const Repository &repo, const string& name) {
        Commit otherHead = get_commit(repo, name);
        AnnotatedCommit annOther = repo.lookup_annotated_commit(otherHead.id());
        vector<AnnotatedCommit> sources;
        sources.push_back(annOther);

        return repo.merge_analysis(sources);
    }

    void reset_head(const Repository& repo, const Commit& commit, bool hard) {
        if (hard) {
            // Changes must be staged, or else they won't get reverted.
            Index index = add_all(repo);
            index.write();
        }

        git_checkout_options checkoutOpts = GIT_CHECKOUT_OPTIONS_INIT;
        checkoutOpts.checkout_strategy = GIT_CHECKOUT_FORCE;
        git_reset_t resetType = hard? GIT_RESET_HARD : GIT_RESET_SOFT;

        repo.reset_to_commit(commit, resetType, checkoutOpts);
    }

    StrArray reference_list(const Repository& repo) {
        git_strarray refs;
        int error = git_reference_list(&refs, repo.ptr().get());
        check_error(error);
        return StrArray(&refs);
    }

    void reset_to_empty(const Repository& repo) {
        // We create an empty tree to replace the working directory with
        Treebuilder empty = Treebuilder::create(repo);
        OID oid = empty.write();
        Tree tree = repo.lookup_tree(oid);

        // Then we simply checkout the tree
        git_checkout_options checkoutOpts = GIT_CHECKOUT_OPTIONS_INIT;
        checkoutOpts.checkout_strategy = GIT_CHECKOUT_FORCE;
        repo.checkout_tree(tree, checkoutOpts);
    }

    bool tree_iterator(function<bool(Commit)> pre, function<bool(Commit)> post, Commit commit) {
        bool exit = pre(commit);
        for (auto parent : commit.parents()) {
            if (exit) break;
            exit = tree_iterator(pre, post, parent) | exit;
        }
        exit = post(commit) | exit;
        return exit;
    }
}