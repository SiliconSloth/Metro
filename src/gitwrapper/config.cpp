#include "pch.h"

namespace git {
    Config Config::open_default() {
        git_config *config;
        int err = git_config_open_default(&config);
        check_error(err);
        return Config(config);
    }

    string Config::get_string_buf(const string &name) {
        git_buf buf{nullptr, 0, 0};
        int err = git_config_get_string_buf(&buf, config.get(), name.c_str());
        check_error(err);
        return string(buf.ptr);
    }

    void Config::get_multivar_foreach(const std::string & name, git_config_foreach_cb callback, void *payload) {
        int err = git_config_get_multivar_foreach(config.get(), name.c_str(), nullptr, callback, payload);
        check_error(err);
    }
}