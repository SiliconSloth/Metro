#pragma once

namespace git {
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

        void fetch(const StrArray& refspecs, FetchOps opts);
        void push(StrArray refspecs, PushOptions opts) const;
    };
}