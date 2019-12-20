#include "pch.h"

namespace metro {
    BranchDescriptor::BranchDescriptor(const string& name) : baseName(name) {
        wip = is_wip(name);
        if (wip) {
            baseName = baseName.substr(0, baseName.size() -  WIP_SUFFIX_LENGTH);
        }

        // Look for a version number suffix, if present.
        size_t suffixStart = name.find_last_of('#');
        if (suffixStart != string::npos) {
            // Try to convert the suffix to a non-negative int.
            // An invalid suffix is assumed to be part of the base name.
            int versionSuffix = parse_pos_int(name.substr(suffixStart + 1, name.size() - suffixStart - 1));
            if (versionSuffix >= 0) {
                version = versionSuffix;
                baseName = baseName.substr(0, suffixStart);
            }
        }
    }

    string BranchDescriptor::full_name() const {
        string name = baseName;
        // Don't bother specifying version 0.
        if (version > 0) {
            name += "#" + to_string(version);
        }
        if (wip) {
            name += WIP_SUFFIX;
        }
        return name;
    }

    string to_wip(const string& name) {
        if (is_wip(name)) {
            return name;
        } else {
            return name + WIP_SUFFIX;
        }
    }

    bool is_wip(const string& name) {
        return has_suffix(name, WIP_SUFFIX);
    }
}
