/*
 * A wrapper for the git_config type.
 */

#pragma once

namespace git {
    /**
     * Memory representation of a set of config files.
     */
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

        /**
         * Open the global, XDG and system configuration files.
         * Utility wrapper that finds the global, XDG and system configuration files and opens them into a single
         * prioritized config object that can be used when accessing default config data outside a repository.
         *
         * @return Default git config.
         */
        static Config open_default();

        /**
         * Get the value of a string config variable.
         * The value of the config will be copied into the buffer.
         * All config files will be looked into, in the order of their defined level.
         * A higher level means a higher priority. The first occurrence of the variable will be returned here.
         *
         * @param name Variable name.
         * @return Value of variable.
         */
        string get_string_buf(const string& name);

        /**
         * Get each value of a multivar in a foreach callback
         * The callback will be called on each variable found
         * The regular expression is applied case-sensitively on the normalized form of the variable name:
         * the section and variable parts are lower-cased. The subsection is left unchanged.
         *
         * @param name Variable name.
         * @param callback The function to be called on each value of the variable.
         * @param payload Opaque pointer to pass to the callback.
         */
        void get_multivar_foreach(const string& name, git_config_foreach_cb callback, void *payload);
    };
}