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

            argv(int argc, char ** argv) {
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

            auto find_parameter(const std::string & name) const {
                return std::find(contents.begin(), contents.end(), name);
            }
            auto find_value_for(const std::string & name) const {
                auto res = find_parameter(name);
                if (res == end()) return res;
                return ++res;
            }

            bool contains(const std::string & name) const {
                return find_parameter(name) != contents.end();
            }
        };
    }
}

#endif

