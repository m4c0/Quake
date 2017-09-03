#ifndef QUAKE_COMMON_HPP
#define QUAKE_COMMON_HPP

#include <string>
#include <vector>

namespace quake {
    namespace common {
        class argv {
        private:
            std::vector<std::string> contents;

        public:
            static std::unique_ptr<argv> current;

            argv(const std::vector<std::string> & args) : contents(args) {
            }
            argv(int argc, char ** argv) : contents() {
                for (int i = 1; i < argc; i++) {
                    contents.push_back(argv[i]);
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

