// A version of memset that won't be optimized away by the compiler.
// Used for overwriting sensitive memory for security reasons.
typedef void* (*memset_t)(void*, int, size_t);
static volatile memset_t memset_volatile = memset;

namespace metro {
    struct CredentialPayload {
        git_cred **credentials;
        const Repository *repo;
    };

    /*
     * Callback for Git operations that request credentials.
     * Tries to get credentials from each helper specified in the config in turn,
     * then defaults to manual credential entry. The obtained credentials pointer is written
     * back to the location specified in the payload as well as the normal cred parameter.
     *
     * If the credentials pointer in the payload is already non-null it is returned
     * instead of acquiring new credentials, allowing credential reuse.
     */
    int acquire_credentials(git_cred **cred, const char *url, const char *username_from_url,
            unsigned int allowed_types, void *payload);

    /*
     * Iterate over the helpers specified in a repo's config until one of them
     * successfully provides credentials for the specified URL.
     */
    void credentials_from_helper(const Repository  *repo, const string& url, git_cred **credentials);

    /*
     * Try to obtain credentials for the specified URL from the specified credential helper.
     * The helper name should be in config value format.
     */
    void credentials_from_helper(const string& helper, const string& url, git_cred **credentials);

    /*
     * Request credentials from the user on the command line.
     */
    int manual_credential_entry(git_cred **cred, const char *url, unsigned int allowed_types);
}