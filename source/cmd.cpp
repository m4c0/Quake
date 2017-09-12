#include "quake/cmd.hpp"
#include "quake/legacy.hpp"

#include <map>
#include <typeindex>

static auto & _cmds() {
    static std::map<std::string, quake::cmd::base *> container;
    return container;
}

static auto _to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

void quake::cmd::install(const std::string & name, quake::cmd::base * cmd) {
    auto lcname = _to_lower(name);

    auto & all = _cmds();
    if (all.find(lcname) != all.end()) {
        throw std::invalid_argument("Duplicated command: " + lcname);
    }
    all[lcname] = cmd;
}

quake::cmd::base * quake::cmd::by_name(const std::string & n) {
    return _cmds().at(_to_lower(n));
}

const char * quake::cmd::complete(const std::string & name) {
    if (name.size() == 0) return nullptr;

    auto lcname = _to_lower(name);

    for (auto & kv : _cmds()) {
        if (kv.first.compare(0, lcname.size(), lcname) == 0) {
            return kv.first.c_str();
        }
    }

    return nullptr;
}

void quake::cmd::find_by_type(const std::type_info & type, std::function<void(base *)> callback) {
    for (auto cmd : _cmds()) {
        if (std::type_index(typeid(*cmd.second)) == std::type_index(type)) {
            callback(cmd.second);
        }
    }
}

void quake::cmd::execute(const char * text) {
    quake::common::argv args { text };

    if (args.cmd == "") return;

    try {
        quake::cmd::by_name(args.cmd)->execute(args);
    } catch (...) {
        Con_Printf ("Unknown command \"%s\"\n", args.cmd.c_str());
    }
}
