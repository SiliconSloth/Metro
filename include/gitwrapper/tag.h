/*
 * Contains a wrapper for the git_tag object.
 */

#pragma once

namespace git {
    class Object;

    class Tag {
    private:
        shared_ptr<git_tag> tag;

    public:
        Tag() : tag(nullptr) {}

        explicit Tag(git_tag *tag) : tag(tag, git_tag_free) {}

        explicit Tag(shared_ptr<git_tag> tag) : tag(std::move(tag)) {}

        [[nodiscard]] shared_ptr<git_tag> ptr() const {
            return tag;
        }

        /**
         * Get the tagged object of a tag.
         *
         * @return The target.
         */
        [[nodiscard]] Object target() const;
    };
}