#include <utility>

#include "pch.h"

namespace metro {
    BranchDescriptor::BranchDescriptor(string  name) : baseName(std::move(name)) {
        wip = is_wip(baseName);
        if (wip) {
            baseName = baseName.substr(0, baseName.size() -  WIP_SUFFIX_LENGTH);
        }

        // Look for a version number suffix, if present.
        size_t suffixStart = baseName.find_last_of('#');
        if (suffixStart != string::npos) {
            // Try to convert the suffix to a non-negative int.
            // An invalid suffix is assumed to be part of the base name.
            int versionSuffix = parse_pos_int(baseName.substr(suffixStart + 1, baseName.size() - suffixStart - 1));
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

    string un_wip(const string& name) {
        if (is_wip(name)) {
            return name.substr(0, name.size() - WIP_SUFFIX_LENGTH);
        } else {
            return name;
        }
    }

    bool is_wip(const string& name) {
        return has_suffix(name, WIP_SUFFIX);
    }
}
