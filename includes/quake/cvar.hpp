#ifndef QUAKE_CVAR_HPP
#define QUAKE_CVAR_HPP

#include "quake/cmd.hpp"
#include "quake/legacy.hpp"

#include <string>

namespace quake {
    namespace cvar {
        class normal : public quake::cmd::base {
        public:
            normal(const std::string & name, const std::string svalue) : base(), name(name), string(svalue) {
                quake::cmd::install(name, this);
            }

            void execute(const quake::common::argv & args) override {
                if (args.size() == 0) {
                    Con_Printf("'%s' is '%s'\n", name.c_str(), string.c_str());
                } else {
                    string = args[0];
                }
            }

            virtual void operator=(const std::string & value) {
                this->string = value;
            }
            virtual void operator=(float value) {
                *this = std::to_string(value);
            }

            bool operator==(const std::string & o) const {
                return string == o;
            }
            bool operator!=(const std::string & o) const {
                return string != o;
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
            const char * to_cstr() const {
                return string.c_str();
            }

            const std::string name;

        protected:
            std::string string;
        };
    }
}

#endif

