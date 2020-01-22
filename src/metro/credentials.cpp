#include "pch.h"

namespace metro {
    // Overwrite the memory occupied by a string with zeros to prevent malicious access.
    void erase_string(string& str) {
        memset_volatile(&str[0], 0, str.size());
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

    void get_keys(string *pub, string *pri) {
#ifdef _WIN32
        cout << "Metro currently doesn't support SSH on Windows. Please use HTTPS." << endl;
        return;
#elif __unix__
        const char* home;
        home = getenv("HOME");

        if (home == NULL) {
            home = getpwuid(getuid())->pw_dir;
        }

        ifstream keyfile;
        keyfile.open(string(home) + "/.ssh/id_rsa.pub");
        if (keyfile.is_open()) {
            string line;
            while (getline(keyfile, line)) {
                pub->append(line);
            }
            keyfile.close();
        } else {
            cout << "Public Key not found at " + string(home) + "/.ssh/id_rsa.pub" << endl;
        }
        keyfile.open(string(home) + "/.ssh/id_rsa");
        if (keyfile.is_open()) {
            string line;
            while (getline(keyfile, line)) {
                pri->append(line);
            }
            keyfile.close();
        } else {
            cout << "Public Key not found at " + string(home) + "/.ssh/id_rsa" << endl;
        }
#endif //_WIN32
    }

    int acquire_credentials(git_cred **cred, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload)  {
        int err = GIT_OK;
        auto credPayload = static_cast<CredentialPayload*>(payload);
        git_cred **credentials = credPayload->credentials;

        if (*credentials == nullptr) {
            credentials_from_helper(credPayload->repo, string(url), credentials);
        }

        if (*credentials == nullptr) {
            err = manual_credential_entry(credentials, url, allowed_types);
        }

        *cred = *credentials;
        return err;
    }

    struct HelperForeachPayload {
        const string *url;
        git_cred **credentials;
    };

    void credentials_from_helper(const Repository *repo, const string& url, git_cred **credentials) {
        try {
            // Iterate over the helpers specified in config.
            Config config = repo == nullptr? Config::open_default() : repo->config();
            HelperForeachPayload payload{&url, credentials};
            config.get_multivar_foreach("credential.helper", [](const git_config_entry *entry, void *payload) {
                auto hfp = static_cast<HelperForeachPayload *>(payload);
                // Skip the remaining helpers once one of them has provided credentials.
                if (*hfp->credentials == nullptr) {
                    credentials_from_helper(entry->value, *hfp->url, hfp->credentials);
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

    void credentials_from_helper(const string& helper, const string& url, git_cred **credentials) {
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
                    } else if (strnequal(helperOut.c_str() + lineStart, "password", equals - lineStart) && username.empty()) {
                        password = helperOut.substr(equals+1, lineEnd - equals - 1);
                    }
                    //TODO: Support quit
                }
            }

            int err = git_cred_userpass_plaintext_new(credentials, username.c_str(), password.c_str());
            check_error(err);
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

    int manual_credential_entry(git_cred **cred, const char *url, unsigned int allowed_types) {
        int err = GIT_OK;

        string username;
        string password;
        string publicKey, privateKey;

        switch (allowed_types) {
            case GIT_CREDTYPE_DEFAULT:
                err = git_cred_default_new(cred);
                break;
            case GIT_CREDTYPE_USERPASS_PLAINTEXT:
                cout << "Username for " << url << ": ";
                getline(cin, username);
                cout << "Password for " << username << ": ";
                read_password(password);

                err = git_cred_userpass_plaintext_new(cred, username.c_str(), password.c_str());
                break;
            default:
                cout << "Username for " << url << ": ";
                getline(cin, username);
                cout << "SSH Keystore Password: ";
                read_password(password);

                get_keys(&publicKey, &privateKey);
//                cout << "Metro currently doesn't support SSH. Please use HTTPS." << endl;
                cout << "Public key is:\n" << publicKey << endl;
                cout << "Private key is:\n" << privateKey << endl;
                err = git_cred_ssh_key_new(cred, username.c_str(), publicKey.c_str(), privateKey.c_str(), password.c_str());
                break;
        }

        // Erase sensitive information from memory.
        erase_string(username);
        erase_string(password);
        erase_string(publicKey);
        erase_string(privateKey);

        return err;
    }
}
