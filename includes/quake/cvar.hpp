#ifndef QUAKE_CVAR_HPP
#define QUAKE_CVAR_HPP

#include "quake/legacy.hpp"

#include <string>

namespace quake {
    namespace cvar {
        class normal {
        public:
            normal(const std::string & name, const std::string svalue, bool archive = false) : name(name), string(svalue), archive(archive) {
                try {
                    value = std::stof(svalue);
                } catch (...) {
                    value = 0;
                }

                install(this);
            }
            ~normal() {
                uninstall(this);
            }

            virtual normal & operator=(const std::string & value) {
                this->string = value;
                try {
                    this->value = std::stof(value);
                } catch (std::invalid_argument) {
                    this->value = 0;
                }
                return *this;
            }
            normal & operator=(float value) {
                return *this = std::to_string(value);
            }

            const std::string name;
            std::string string;
            bool archive;
            float value;

        private:
            static void install(normal * v);
            static void uninstall(normal * v);
        };

        normal & by_name(const std::string & name);
        const char * complete(const std::string &);
        void persist(FILE * f);
    }
}

#endif

