/*
 * Contains wrapper for git_object.
 */

#pragma once

namespace git {
    /**
     * Representation of a generic object in a repository
     */
    class Object {
    private:
        shared_ptr<git_object> object;

    public:
        explicit Object(git_object *object) : object(object, git_object_free) {}

        Object() = delete;

        Object operator=(Object o) = delete;

        [[nodiscard]] shared_ptr<git_object> ptr() const {
            return object;
        }

        /**
         * Get the object type of an object.
         *
         * @return The object's type.
         */
        [[nodiscard]] git_object_t type() const {
            return git_object_type(object.get());
        }

        explicit operator Commit() {
            return Commit(shared_ptr<git_commit>(object, reinterpret_cast<git_commit*>(object.get())));
        }

        explicit operator Tag() {
            return Tag(shared_ptr<git_tag>(object, reinterpret_cast<git_tag*>(object.get())));
        }
    };
}