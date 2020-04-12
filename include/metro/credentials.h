/*
 * This file contains code regarding credential handling.
 */

#pragma once
#include <utility>

// A version of memset that won't be optimized away by the compiler.
// Used for overwriting sensitive memory for security reasons.
typedef void* (*memset_t)(void*, int, size_t);
static volatile memset_t memset_volatile = memset;

namespace metro {
    /**
     * Represents supported credential types
     */
    enum CredentialType{EMPTY, DEFAULT, USERPASS, SSH_KEY};

    /**
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
        // Whether those credentials have been tried and proved invalid
        bool tried = false;

        /**
         * Sets the store type to use whatever the default is.
         */
        void store_default();

        /**
         * Sets the store type to require a username and a password.
         * @param username Username to enter into store.
         * @param password Password to enter into store.
         */
        void store_userpass(string username, string password);

        /**
         * Sets the store type to SSH, requiring valid SSH keys
         * @param username URL username (usually "git").
         * @param password Password for SSH private key.
         * @param publicKey Path to public key.
         * @param privateKey Path to private key.
         */
        void store_ssh_key(string username, string password, string publicKey, string privateKey);

        /**
         * Checks if the store is currently empty.
         * @return True if store is empty.
         */
        bool empty() {
            return type == EMPTY;
        };

        /**
         * Create git_cred object from the information in this store.
         * @param cred Credential object to convert to.
         * @returns Git error code.
         */
        int to_git(git_cred **cred);

        /**
         * Clears the store of the current contents
         */
        void clear();

        ~CredentialStore();
    };

    // Tuple of Credential Store and Repository
    struct CredentialPayload {
        CredentialStore *credStore;
        const Repository *repo;
    };

    // Tuple of URL and Credential Store
    struct HelperForeachPayload {
        const string *url;
        CredentialStore *credStore;
    };

    /**
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

    /**
     * Iterate over the helpers specified in a repo's config until one of them
     * successfully provides credentials for the specified URL.
     *
     * @param repo The repository to use for config.
     * @param url The URL to connect to.
     * @param credStore Credential store used to put credentials found into.
     */
    void credentials_from_helper(const Repository *repo, const string& url, CredentialStore& credStore);

    /**
     * Try to obtain credentials for the specified URL from the specified credential helper.
     * The helper name should be in config value format.
     *
     * @param helper Which helper to use.
     * @param url The URL to connect to.
     * @param credStore Credential store used to put credentials found into.
     */
    void credentials_from_helper(const string& helper, const string& url, CredentialStore& credStore);

    /**
     * Try to retrieve a user-specified command to use for requesting passwords from Git's usual locations.
     * First try the GIT_ASKPASS environment variable, then the core.askPass config value,
     * then the SSH_ASKPASS environment variable.
     * If no value could be found an empty string is returned.
     *
     * @param repo The repository to use the config for, or nullptr to use default.
     * @return The user-specified command for requesting passwords from git, or an empty string if none found.
     */
    string get_askpass_cmd(const Repository *repo);

    /**
     * Prompt the user for input using the specified askpass command.
     * If the command fails or an empty command string is provided, defaults to terminal entry.
     * If isPassword is set then terminal entry will hide the user's input.
     *
     * @param cmd Command to prompt the user using.
     * @param prompt Prompt to present to the user.
     * @param isPassword Whether the input is for a password and should not be shown visually.
     * @param out The input the user inputted into the prompt.
     */
    void read_from_askpass(const string& cmd, const string& prompt, bool isPassword, string& out);

    /**
     * Request credentials from the user on the command line.
     *
     * @param repo The repo to use to decide what credential type to use.
     * @param url The url for the manual credential entry.
     * @param username_from_url The username receieved from the url.
     * @param allowed_types Types of credential input that can be used.
     * @param credStore The credential store the result will be put into.
     */
    void manual_credential_entry(const Repository *repo, const char *url, const char *username_from_url, unsigned int allowed_types, CredentialStore& credStore);
}