#ifndef QUAKE_CVAR_SERVERSIDE_HPP
#define QUAKE_CVAR_SERVERSIDE_HPP

#include "quake/cvar.hpp"

#include <list>

namespace quake {
    namespace cvar {
        class server_side : public normal {
        public:
            static std::list<server_side *> & all() {
                static std::list<quake::cvar::server_side *> list;
                return list;
            }

            server_side(const std::string & name, const std::string svalue) : normal(name, svalue) {
                all().push_back(this);
            }

            void operator=(const std::string & value) override {
                bool changed = this->string != value;

                normal::operator=(value);

                if (changed) {
                    SV_BroadcastPrintf ("\"%s\" changed to \"%s\"\n", this->name.c_str(), this->string.c_str());
                }
            }
            void operator=(float value) override {
                *this = std::to_string(value);
            }
            bool operator==(const std::string & o) const {
                return normal::operator==(o);
            }
            bool operator!=(const std::string & o) const {
                return normal::operator!=(o);
            }
        };
    }
}

#endif

