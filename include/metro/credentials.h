// A version of memset that won't be optimized away by the compiler.
// Used for overwriting sensitive memory for security reasons.
typedef void* (*memset_t)(void*, int, size_t);
static volatile memset_t memset_volatile = memset;

namespace metro {
    int acquire_credentials(git_cred **cred, const char *url, const char *username_from_url,
            unsigned int allowed_types, void *payload);
}