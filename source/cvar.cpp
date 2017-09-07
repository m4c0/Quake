#include "quake/cvar.hpp"

#include <map>
#include <memory>

static auto & _cvars() {
    static std::map<std::string, quake::cvar *> _cvars;
    return _cvars;
}

void quake::cvar::install(quake::cvar * v) {
    _cvars()[v->name] = v;
}
void quake::cvar::uninstall(quake::cvar * v) {
    _cvars().erase(v->name);
}

quake::cvar & quake::cvar::by_name(const std::string & n) {
    return *(_cvars().at(n));
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

void quake::cvar::persist(FILE * f) {
    for (auto & kv : _cvars()) {
        auto v = kv.second;
        if (v->archive) {
			fprintf (f, "%s \"%s\"\n", v->name.c_str(), v->string.c_str());
        }
    }
}

quake::cvar * quake::cvar::first() {
    return _cvars().begin()->second;
}
quake::cvar * quake::cvar::next() const {
    auto it = std::find_if(_cvars().begin(), _cvars().end(), [this](auto & kv) { return kv.second == this; });
    it++;
    return (it == _cvars().end()) ? nullptr : it->second;
}

