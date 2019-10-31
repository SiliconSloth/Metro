#pragma once

#define OID_LENGTH GIT_OID_HEXSZ+1

namespace git {
    class OID {
    public:
        git_oid oid;

        explicit OID(git_oid oid) : oid(oid) {}

        [[nodiscard]] string str() const {
            char out[OID_LENGTH];
            git_oid_tostr(out, OID_LENGTH, &oid);
            return string(out);
        }
    };
}
