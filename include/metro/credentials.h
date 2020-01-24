#include <utility>

// A version of memset that won't be optimized away by the compiler.
// Used for overwriting sensitive memory for security reasons.
typedef void* (*memset_t)(void*, int, size_t);
static volatile memset_t memset_volatile = memset;

namespace metro {
    enum CredentialType{EMPTY, DEFAULT, USERPASS, SSH_KEY};

    /*
     * Store credentials of various types so that git_cred objects
     * can be generated from them.
     * Starts empty and is initialized as needed with store methods.
     */
    class CredentialStore {
        CredentialType type = EMPTY;

        string username;
        string password;
        string publicKey;
        string privateKey;

    public:
        void store_default();

        void store_userpass(string username, string password);

        void store_ssh_key(string username, string password, string publicKey, string privateKey);

        bool empty() {
            return type == EMPTY;
        };

        /*
         * Create git_cred object from the information in this store.
         * Returns a Git error code.
         */
        int to_git(git_cred **cred);

        ~CredentialStore();
    };

    int acquire_credentials(git_cred **cred, const char *url, const char *username_from_url,
            unsigned int allowed_types, void *payload);
}