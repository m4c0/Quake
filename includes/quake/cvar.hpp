#ifndef QUAKE_CVAR_HPP
#define QUAKE_CVAR_HPP

#include "quake/legacy.hpp"

#include <string>

namespace quake {
    namespace cvar {
        class normal {
        public:
            normal(const std::string & name, const std::string svalue) : name(name), string(svalue) {
                install(this);
            }
            ~normal() {
                uninstall(this);
            }

            virtual void operator=(const std::string & value) {
                this->string = value;
            }
            virtual void operator=(float value) {
                *this = std::to_string(value);
            }

            bool to_bool() const {
                return this->to_int() != 0;
            }
            int to_int() const {
                return (int)this->to_float(); // an extra stoi+catch looked messier
            }
            float to_float() const {
                try {
                    return std::stof(string);
                } catch (std::invalid_argument) {
                    return 0;
                }
            }

            const std::string name;
            std::string string;

        private:
            static void install(normal * v);
            static void uninstall(normal * v);
        };

        normal * by_name(const std::string & name);
        const char * complete(const std::string &);
    }
}

#endif

