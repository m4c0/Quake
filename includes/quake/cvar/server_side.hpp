/*
 * m4c0's tweaks on Quake - Quake rewritten in C++17/OpenGL 3.3
 * Copyright (C) 2017 Eduardo Costa
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

