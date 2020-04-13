/*
 * Contains a wrapper for the git_status_list object.
 */

#pragma once

namespace git {
    /**
     * Representation of a status collection
     */
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

        /**
         * Gets the count of status entries in this list.
         *
         * If there are no changes in status (at least according the options given
         * when the status list was created), this can return 0.
         *
         * @return the number of status entries
         */
        [[nodiscard]] size_t entrycount() const {
            return git_status_list_entrycount(status.get());
        }
    };
}