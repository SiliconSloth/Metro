/*
 * Defines a wrapper for the git_annotated_commit type.
 */

#pragma once

namespace git {
    /**
     * Annotated commits, the input to merge and rebase.
     */
    class AnnotatedCommit {
    private:
        shared_ptr<git_annotated_commit> commit;

    public:
        explicit AnnotatedCommit(git_annotated_commit *commit) : commit(commit, git_annotated_commit_free) {}

        explicit AnnotatedCommit(shared_ptr<git_annotated_commit> commit) : commit(std::move(commit)) {}

        AnnotatedCommit() = delete;

        AnnotatedCommit operator=(AnnotatedCommit c) = delete;

        [[nodiscard]] shared_ptr<git_annotated_commit> ptr() const {
            return commit;
        }
    };
}