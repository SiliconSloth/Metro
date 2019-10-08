#pragma once

namespace git {
    class StatusList {
    private:
        shared_ptr<git_status_list> status;

    public:
        explicit StatusList(git_status_list *status) : status(status, git_status_list_free) {}

        StatusList() = delete;

        StatusList operator=(StatusList s) = delete;

        [[nodiscard]] shared_ptr<git_status_list> ptr() const {
            return status;
        }

        [[nodiscard]] int entrycount() const {
            return git_status_list_entrycount(status.get());
        }
    };
}