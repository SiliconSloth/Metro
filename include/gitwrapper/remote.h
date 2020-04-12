/*
 * Contains wrapper for git_remote type.
 */

#pragma once

namespace git {
    /**
     * Git's idea of a remote repository. A remote can be anonymous (in which case
     * it does not have backing configuration entires).
     */
    class Remote {
    private:
        shared_ptr<git_remote> remote;

    public:
        explicit Remote(git_remote *remote) : remote(remote, git_remote_free) {}

        Remote() = delete;

        Remote operator=(Remote c) = delete;

        [[nodiscard]] shared_ptr<git_remote> ptr() const {
            return remote;
        }

        /**
         * Download new data and update tips
         *
         * Convenience function to connect to a remote, download the data,
         * disconnect and update the remote-tracking branches.
         *
         * @param refspecs The refspecs to use for this fetch.
         * @param opts Options to use for this fetch.
         */
        void fetch(const StrArray& refspecs, FetchOps opts);

        /**
         * Perform a push
         *
         * Peform all the steps from a push.
         *
         * @param refspecs The refspecs to use for pushing.
         * @param opts Options to use for this push.
         */
        void push(StrArray refspecs, PushOptions opts) const;
    };
}