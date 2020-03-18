#include <utility>

// A version of memset that won't be optimized away by the compiler.
// Used for overwriting sensitive memory for security reasons.
typedef void* (*memset_t)(void*, int, size_t);
static volatile memset_t memset_volatile = memset;

namespace metro {
    enum CredentialType{EMPTY, DEFAULT, USERPASS, SSH_KEY, RAW};

    /*
     * Store credentials of various types so that git_cred objects
     * can be generated from them.
     * Starts empty and is initialized as needed with store methods.
     */
    class CredentialStore {
        CredentialType type = EMPTY;
        git_cred *cred;

        string username;
        string password;
        string publicKey;
        string privateKey;

    public:
        bool tried = false;

        void store_default();

        void store_userpass(string username, string password);

        void store_ssh_key(string username, string password, string publicKey, string privateKey);

        void store_raw(git_cred *cred);

        bool empty() {
            return type == EMPTY;
        };

        /*
         * Create git_cred object from the information in this store.
         * Returns a Git error code.
         */
        int to_git(git_cred **cred);

        void clear();

        ~CredentialStore();
    };

    struct CredentialPayload {
        CredentialStore *credStore;
        const Repository *repo;
    };

    struct HelperForeachPayload {
        const string *url;
        CredentialStore *credStore;
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
    void credentials_from_helper(const Repository  *repo, const string& url, CredentialStore& credStore);

    /*
     * Try to obtain credentials for the specified URL from the specified credential helper.
     * The helper name should be in config value format.
     */
    void credentials_from_helper(const string& helper, const string& url, CredentialStore& credStore);

    // Try to retrieve a user-specified command to use for requesting passwords from Git's usual locations.
    // First try the GIT_ASKPASS environment variable, then the core.askPass config value,
    // then the SSH_ASKPASS environment variable.
    // If no value could be found an empty string is returned.
    string get_askpass_cmd(const Repository *repo);

    // Prompt the user for input using the specified askpass command.
    // If the command fails or an empty command string is provided, defaults to terminal entry.
    // If isPassword is set then terminal entry will hide the user's input.
    void read_from_askpass(const string& cmd, const string& prompt, bool isPassword, string& out);

    /*
     * Request credentials from the user on the command line.
     */
    void manual_credential_entry(const Repository *repo, const char *url, const char *username_from_url, unsigned int allowed_types, CredentialStore& credStore);
}