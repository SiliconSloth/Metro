#pragma once

#define OID_LENGTH GIT_OID_HEXSZ+1

namespace git {
    class OID {
    public:
        git_oid oid;
        bool isNull = false;

        explicit OID(git_oid oid) : oid(oid) {}
        explicit OID() : isNull(true) {}

        bool operator==(const OID& other) const;
        bool operator!=(const OID& other) const;

        [[nodiscard]] string str() const;
    };
}
