#pragma once

#define OID_LENGTH GIT_OID_HEXSZ+1

namespace git {
    class OID {
    public:
        git_oid oid;
        bool isNull = false;

        explicit OID(git_oid oid) : oid(oid) {}
        explicit OID() : isNull(true) {}

        [[nodiscard]] string str() const {
            if (isNull) return "";
            char out[OID_LENGTH];
            git_oid_tostr(out, OID_LENGTH, &oid);
            return string(out);
        }
    };
}
