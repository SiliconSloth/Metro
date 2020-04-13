namespace metro {
    UrlDescriptor::UrlDescriptor(const string &url) {
        string rem = url;
        // Treat all types of slashes the same.
        replace(rem.begin(), rem.end(), '\\', '/');

        // Remove the protocol if present.
        size_t protocolEnd = rem.find("://");
        if (protocolEnd != string::npos) {
            protocol = rem.substr(0, protocolEnd);
            rem = rem.substr(protocolEnd + 3, rem.size() - (protocolEnd + 3));
        }

        // If a username is provided in the form user@server/something/repo, remove it.
        const regex authPattern("^[^\\/]*@");
        smatch matches;
        regex_search(rem, matches, authPattern);
        // Since the regex starts with ^ there should be at most one match.
        for (auto match : matches) {
            rem = rem.substr(match.length(), rem.size() - match.length());
            break;
        }

        // Parse hostname and path without removing them.
        size_t hostEnd = rem.find('/');
        if (hostEnd != string::npos) {
            host = rem.substr(0, hostEnd);
            path = rem.substr(hostEnd + 1, rem.size() - (hostEnd + 1));
        } else {
            path = rem;
        }

        // Get the final component of the URL path, ignoring any empty/whitespace only components.
        // If the last non-empty component is ".git" it will be ignored, but any prior ".git"s
        // will be kept.
        size_t lastSlash;
        string lastComponent;
        bool skippedGit = false;
        while (true) {
            lastSlash = rem.find_last_of('/');
            if (lastSlash != string::npos) {
                lastComponent = rem.substr(lastSlash + 1, rem.size() - (lastSlash + 1));
                rem = rem.substr(0, lastSlash);
            } else {
                // Stop searching once the first component of the URL is reached,
                // even if it is invalid (that will be checked later).
                break;
            }

            // If this component is suitable, stop searching.
            if (!whitespace_only(lastComponent) && (lastComponent != ".git" || skippedGit)) {
                rem = lastComponent;
                break;
            }
            // Only skip the first ".git".
            if (lastComponent == ".git") {
                skippedGit = true;
            }
        }

        // If the repo name ends with ".git" or ".bundle", but this is not the entire name,
        // remove the extension.
        if (has_suffix(rem, ".git") && rem != ".git") {
            rem = rem.substr(0, rem.size() - 4);
        } else if (has_suffix(rem, ".bundle") && rem != ".bundle") {
            rem = rem.substr(0, rem.size() - 7);
        }

        if (!whitespace_only(rem)) {
            repository = rem;
        }
    }
}