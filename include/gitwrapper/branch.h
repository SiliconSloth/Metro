#pragma once

namespace git {
    class Branch {
    private:
        shared_ptr<git_reference> ref;

    public:
        explicit Branch(git_reference *ref) : ref(ref, git_reference_free) {}

        Branch() = default;

        [[nodiscard]] shared_ptr<git_reference> ptr() const {
            return ref;
        }

        [[nodiscard]] string name() const;
        [[nodiscard]] bool is_head() const;
        void delete_branch();
        OID target();
        void set_target(OID, const char *);

        [[nodiscard]] string reference_name() const;
    };
}