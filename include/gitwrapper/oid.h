/*
 * Contains a wrapper for git_oid
 */

#pragma once

#define OID_LENGTH GIT_OID_HEXSZ+1

namespace git {
    class OID {
    public:
        git_oid oid;
        bool isNull = false;

        explicit OID(git_oid oid) : oid(oid) {}
        explicit OID() : isNull(true) {}
        explicit OID(const string& str);

        bool operator==(const OID& other) const;
        bool operator!=(const OID& other) const;

        bool operator<(const OID& other) const;

        /**
         * Format an OID into a buffer as a string.
         *
         * @return OID string.
         */
        [[nodiscard]] string str() const;
    };
}
