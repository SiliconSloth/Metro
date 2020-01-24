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

    CredentialStore::~CredentialStore() {
        erase_string(username);
        erase_string(password);
        erase_string(publicKey);
        erase_string(privateKey);
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

    int acquire_credentials(git_cred **cred, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload) {
        auto credStore = static_cast<CredentialStore *>(payload);
        if (credStore->empty()) {
            string username;
            string password;
            string publicKey, privateKey;

            switch (allowed_types) {
                case GIT_CREDTYPE_DEFAULT:
                    credStore->store_default();
                    break;
                case GIT_CREDTYPE_USERPASS_PLAINTEXT:
                    cout << "Username for " << url << ": ";
                    getline(cin, username);
                    cout << "Password for " << username << ": ";
                    read_password(password);

                    credStore->store_userpass(username, password);
                    break;
                default:
                    cout << "Username for " << url << ": ";
                    getline(cin, username);
                    cout << "SSH Keystore Password: ";
                    read_password(password);

                    get_keys(&publicKey, &privateKey);
                    credStore->store_ssh_key(username, password, publicKey, privateKey);
                    break;
            }

            // Erase sensitive information from memory.
            erase_string(username);
            erase_string(password);
            erase_string(publicKey);
            erase_string(privateKey);
        }

        return credStore->to_git(cred);
    }
}
