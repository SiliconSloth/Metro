#pragma once

namespace git {
    class Config {
    private:
        shared_ptr<git_config> config;

    public:
        explicit Config(git_config *config) : config(config, git_config_free) {}

        explicit Config(shared_ptr<git_config> config) : config(std::move(config)) {}

        Config() = delete;

        Config operator=(Config d) = delete;

        [[nodiscard]] shared_ptr<git_config> ptr() const {
            return config;
        }

        static Config open_default();

        string get_string_buf(const string& name);

        void get_multivar_foreach(const string& name, git_config_foreach_cb callback, void *payload);
    };
}