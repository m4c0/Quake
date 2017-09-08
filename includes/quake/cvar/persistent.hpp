#ifndef QUAKE_CVAR_PERSISTENT_HPP
#define QUAKE_CVAR_PERSISTENT_HPP

#include "quake/cvar.hpp"

#include <list>

namespace quake {
    namespace cvar {
        class persistent : public normal {
        public:
            static std::list<persistent *> & all() {
                static std::list<quake::cvar::persistent *> list;
                return list;
            }

            persistent(const std::string & name, const std::string svalue) : normal(name, svalue) {
                all().push_back(this);
            }
        };
    }
}

#endif

