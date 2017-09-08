#include "quake/cvar.hpp"

#include <map>
#include <memory>

static auto & _cvars() {
    static std::map<std::string, quake::cvar::normal *> _cvars;
    return _cvars;
}

void quake::cvar::normal::install(quake::cvar::normal * v) {
    _cvars()[v->name] = v;
}
void quake::cvar::normal::uninstall(quake::cvar::normal * v) {
    _cvars().erase(v->name);
}

quake::cvar::normal * quake::cvar::by_name(const std::string & n) {
    return _cvars().at(n);
}

const char * quake::cvar::complete(const std::string & name) {
    if (name.size() == 0) return nullptr;

    for (auto & kv : _cvars()) {
        if (kv.first.compare(0, name.size(), name) == 0) {
            return kv.first.c_str();
        }
    }

    return nullptr;
}

