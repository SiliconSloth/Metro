namespace git {
    [[nodiscard]] string OID::str() const {
        if (isNull) return "NULL";
        char out[OID_LENGTH];
        git_oid_tostr(out, OID_LENGTH, &oid);
        return string(out);
    }

    bool OID::operator==(const OID& other) const {
        return (isNull && other.isNull) ||
            (!isNull && !other.isNull && git_oid_equal(&oid, &other.oid));
    }

    bool OID::operator!=(const OID& other) const {
        return !operator==(other);
    }
}