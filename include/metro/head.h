#pragma once

namespace metro {
    /**
     * Represents the current head of a repository.
     * name is either a branch name (possibly with preceding remote name) or a commit ID, depending on the head.
     * detached indicates if the head is detached or not.
     */
    struct Head {
        string name;
        bool detached;
    };
}
