#include "pch.h"

namespace metro {
    // Overwrite the memory occupied by a string with zeros to prevent malicious access.
    void erase_string(string& str) {
        memset_volatile(&str[0], 0, str.size());
    }

    void CredentialStore::store_default() {
        assert (type == EMPTY);
        type = DEFAULT;
    }

    void CredentialStore::store_userpass(string username, string password) {
        assert (type == EMPTY);
        type = USERPASS;

        this->username = move(username);
        this->password = move(password);
    }

    void CredentialStore::store_ssh_key(string username, string password, string publicKey, string privateKey) {
        assert (type == EMPTY);
        type = SSH_KEY;

        this->username = move(username);
        this->password = move(password);
        this->publicKey = move(publicKey);
        this->privateKey = move(privateKey);
    }

    int CredentialStore::to_git(git_cred **cred) {
        switch (type) {
            case DEFAULT:
                return git_cred_default_new(cred);
            case USERPASS:
                return git_cred_userpass_plaintext_new(cred, username.c_str(), password.c_str());
            case SSH_KEY:
                return git_cred_ssh_key_new(cred, username.c_str(), publicKey.c_str(), privateKey.c_str(), password.c_str());
            case EMPTY:
                throw MetroException("Can't access empty credential store");
        }
    }

    void CredentialStore::clear() {
        type = EMPTY;
        tried = false;
        erase_string(username);
        erase_string(password);
        erase_string(publicKey);
        erase_string(privateKey);
    }

    CredentialStore::~CredentialStore() {
        clear();
    }

    // Read a password from stdin without displaying the input to the user.
    void read_password(string& out) {
#ifdef _WIN32
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        DWORD mode = 0;
        GetConsoleMode(hStdin, &mode);
        SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
#else
        termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
#endif //_WIN32

        getline(cin, out);
        cout << endl;

#ifdef _WIN32
        SetConsoleMode(hStdin, mode);
#else
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif //_WIN32
    }

    int acquire_credentials(git_cred **cred, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload) {
        auto credPayload = static_cast<CredentialPayload*>(payload);
        CredentialStore *credStore = credPayload->credStore;

        if (credStore->tried) {
            cout << "Invalid credentials, please try again or press Ctrl+C to abort" << endl;
            credStore->clear();
        }

        if (credStore->empty()) {
            credentials_from_helper(credPayload->repo, string(url), *credStore);
        }

        if (credStore->empty()) {
            manual_credential_entry(credPayload->repo, url, allowed_types, *credStore);
        }

        credStore->tried = true;
        return credStore->to_git(cred);
    }

    void credentials_from_helper(const Repository *repo, const string& url, CredentialStore& credStore) {
        try {
            // Iterate over the helpers specified in config.
            Config config = repo == nullptr? Config::open_default() : repo->config();
            HelperForeachPayload payload{&url, &credStore};
            config.get_multivar_foreach("credential.helper", [](const git_config_entry *entry, void *payload) {
                auto hfp = static_cast<HelperForeachPayload *>(payload);
                // Skip the remaining helpers once one of them has provided credentials.
                if (hfp->credStore->empty()) {
                    credentials_from_helper(entry->value, *hfp->url, *hfp->credStore);
                }
                return 0;
            }, &payload);
        } catch (exception& e) {
            // Rather than crashing on exceptions, just print the error message and default to manual credential entry.
            // Don't even print the message if the error was just that no helper was specified.
            if (strcmp(e.what(), "config value 'credential.helper' was not found") != 0) {
                cout << e.what() << endl;
            }
        }
    }

    void credentials_from_helper(const string& helper, const string& url, CredentialStore& credStore) {
        string helperOut, helperErr;
        string username, password;

        try {
            // Convert the helper config value to the command to run, using Git's format.
            string helperCmd;
            if (has_prefix(helper, "!")) {
                helperCmd = helper.substr(1, helper.size() - 1);
            } else if (std::experimental::filesystem::path(helper).is_absolute()) {
                helperCmd = helper;
            } else {
                helperCmd = "git credential-" + helper;
            }

            // Convert the URL information into the input format used by Git helper programs.
            UrlDescriptor desc(url);
            string details =
                    "protocol=" + desc.protocol + "\n" +
                    "host=" + desc.host + "\n" +
                    "path=" + desc.path + "\n\n";

            run_command(helperCmd + " get", details, helperOut, helperErr);
            if (!helperErr.empty()) {
                throw MetroException(helperErr);
            }

            // Parse the helper output line by line to get the username and password.
            // We avoid copying strings to make it easier to erase sensitive information.
            size_t lineStart;
            size_t equals = -1;
            size_t lineEnd = -1;
            for (size_t i = 0; i < helperOut.size(); i++) {
                if (helperOut[i] == '=') {
                    equals = i;
                } else if (helperOut[i] == '\n') {
                    lineStart = lineEnd + 1;
                    lineEnd = i;
                    // Skip lines without an equals.
                    if (equals <= lineStart) {
                        continue;
                    }

                    // Check if the current line contains the username or password.
                    // In the unlikely event that more than one username or password is returned,
                    // only keep the first one in order to avoid leaking a string containing sensitive information
                    // when replacing the previous value.
                    if (strnequal(helperOut.c_str() + lineStart, "username", equals - lineStart) && username.empty()) {
                        username = helperOut.substr(equals+1, lineEnd - equals - 1);
                    } else if (strnequal(helperOut.c_str() + lineStart, "password", equals - lineStart) && password.empty()) {
                        password = helperOut.substr(equals+1, lineEnd - equals - 1);
                    }
                    //TODO: Support quit
                }
            }

            credStore.store_userpass(username, password);
        } catch (exception& e) {
            // Just print error messages instead of crashing, so other helpers can be tried.
            cout << e.what() << endl;
        }

        // Securely overwrite any variables that may have held sensitive information.
        erase_string(helperOut);
        erase_string(helperErr);
        erase_string(username);
        erase_string(password);
    }

    string get_askpass_cmd(const Repository *repo) {
        string cmdStr;
        const char *cmd = getenv("GIT_ASKPASS");
        if (cmd == nullptr) {
            try {
                Config config = repo == nullptr? Config::open_default() : repo->config();
                cmdStr = config.get_string_buf("core.askPass");
                cmd = cmdStr.c_str();
            } catch (exception& e) {
                // Rather than crashing on exceptions, just print the error message and default to terminal entry.
                // Don't even print the message if the error was just that no askPass was specified.
                if (strcmp(e.what(), "config value 'core.askPass' was not found") != 0) {
                    cout << e.what() << endl;
                }
            }
        }

        if (cmd == nullptr) {
            cmd = getenv("SSH_ASKPASS");
        }
        return cmd == nullptr? "" : string(cmd);
        //TODO: Support checking of GIT_TERMINAL_PROMPT
    }

    void read_from_askpass(const string& cmd, const string& prompt, bool isPassword, string& out) {
        // If an askpass command was provided, try using it.
        if (!cmd.empty()) {
            string err;
            try {
                run_command(cmd + " \"" + prompt + ":\"", "", out, err);
            } catch (exception& e) {
                cout << e.what() << endl;
            }
            erase_string(err);
        }

        // Default to terminal-based entry.
        if (out.empty()) {
            cout << prompt << ": ";
            if (isPassword) {
                read_password(out);
            } else {
                getline(cin, out);
            }
        }
    }

    void manual_credential_entry(const Repository *repo, const char *url, unsigned int allowed_types, CredentialStore& credStore) {
        string askpassCmd = get_askpass_cmd(repo);

        string username;
        string password;

        switch (allowed_types) {
            case GIT_CREDTYPE_DEFAULT:
                credStore.store_default();
                break;
            case GIT_CREDTYPE_USERPASS_PLAINTEXT:
                read_from_askpass(askpassCmd, "Username for " + string(url), false, username);
                read_from_askpass(askpassCmd, "Password for " + username, true, password);

                credStore.store_userpass(username, password);
                break;
            default:
                read_from_askpass(askpassCmd, "SSH keystore passphrase: ", true, password);

                const char* home;
#ifdef WIN32
                home = getenv("USERPROFILE");
#else
                home = getenv("HOME");
#endif

#ifdef __unix__
                if (home == NULL) {
                    home = getpwuid(getuid())->pw_dir;
                }
#endif

                //TODO: Don't force these defaults
                credStore.store_ssh_key("git", password, string(home) + "/.ssh/id_rsa.pub", string(home) + "/.ssh/id_rsa");
                break;
        }

        // Erase sensitive information from memory.
        erase_string(username);
        erase_string(password);
    }
}
