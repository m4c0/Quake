#ifndef QUAKE_COMMON_HPP
#define QUAKE_COMMON_HPP

#include "quake/legacy.hpp"

#include <string>
#include <vector>

namespace quake {
    namespace common {
        // TODO: review design/purpose of this class
        // Maybe mixing "process arguments" and "command arguments" is not
        // a good idea. They looked the same since the former was handled
        // by com_argc/com_argv and the latter, by cmd_argc/cmd_argv. Now,
        // it's clear "cmds" require more stuff
        class argv {
        private:
            std::vector<std::string> contents;

        public:
            static std::unique_ptr<const argv> current;

            // FIXME: Provide a better interface, or remove usages
            // Currently, in use by commands with "dynamic parameters".
            // Ex: "kick <user> <msg>"
            std::string all;
            // and this is used by "Cmd_ForwardToServer"
            std::string cmd;

            argv() : contents() {
            }
            argv(const std::vector<std::string> & args) : contents(args) {
            }
            argv(int argc, char ** argv) : contents() {
                for (int i = 1; i < argc; i++) {
                    contents.push_back(argv[i]);
                }
            }
            argv(const char * text) {
                // FIXME: improve this code (formely "Cmd_TokenizeString")
                while (true) {
                    // skip whitespace up to a /n
                    while (*text && *text <= ' ' && *text != '\n') {
                        text++;
                    }

                    if (*text == '\n') {
                        // a newline seperates commands in the buffer
                        text++;
                        break;
                    }

                    if (!*text)
                        return;

                    if (cmd != "")
                        all = text;

                    text = COM_Parse (text);
                    if (!text)
                        return;

                    if (cmd == "") {
                        cmd = com_token;
                    } else {
                        contents.push_back(com_token);
                    }
                }
            }

            auto begin() const {
                return contents.begin();
            }
            auto end() const {
                return contents.end();
            }
            auto size() const {
                return contents.size();
            }

            const std::string & operator[](int idx) const {
                static std::string null = "";
                if (idx >= contents.size()) return null;
                return contents[idx];
            }
            // Allows an easy replacement for old permissive "atoi"
            int stoi(int idx, int def_val) const {
                if (idx >= contents.size()) return def_val;
                try {
                    return std::stoi(contents[idx]);
                } catch (...) {
                    return def_val;
                }
            }
            int stoi(int idx) const {
                return stoi(idx, 0);
            }
            // Allows an easy replacement for old permissive "atof"
            float stof(int idx, float def_val) const {
                if (idx >= contents.size()) return def_val;
                try {
                    return std::stof(contents[idx]);
                } catch (...) {
                    return def_val;
                }
            }
            float stof(float idx) const {
                return stof(idx, 0.0f);
            }

            auto find_parameter(const std::string & name) const {
                return std::find(contents.begin(), contents.end(), name);
            }
            bool contains(const std::string & name) const {
                return find_parameter(name) != contents.end();
            }
            auto get_or_default(const std::string & name, const std::string & def_value) const {
                // TODO: Is "-cachedir" the only one that tests if the value is actually another parameter?
                auto res = find_parameter(name);
                if (res == contents.end()) return def_value;

                res++;
                if (res == contents.end()) return def_value;

                return *res;
            }
        };
    }
}

#endif

