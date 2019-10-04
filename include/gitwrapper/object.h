#pragma once

namespace git {
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

        explicit operator Commit() {
            return Commit(shared_ptr<git_commit>(object, reinterpret_cast<git_commit*>(object.get())));
        }
    };
}