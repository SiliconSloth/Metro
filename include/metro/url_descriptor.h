/*
 * Describes how url's will be handled.
 */

#pragma once

namespace metro {
    struct UrlDescriptor {
        string protocol;
        string host;
        string path;
        string repository;

        explicit UrlDescriptor(const string& url);
    };
}
