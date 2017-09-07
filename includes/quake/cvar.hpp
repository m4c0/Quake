#ifndef QUAKE_CVAR_HPP
#define QUAKE_CVAR_HPP

#include "quake/legacy.hpp"

#include <string>

namespace quake {
    class cvar {
    public:
        static cvar * first();
        static cvar & by_name(const std::string & name);
        static const char * complete(const std::string &);
        static void persist(FILE * f);

        cvar(const std::string & name, const std::string svalue, bool archive = false, bool server = false) : name(name), string(svalue), archive(archive), server(server) {
            try {
                value = std::stof(svalue);
            } catch (...) {
                value = 0;
            }

            install(this);
        }
        ~cvar() {
            uninstall(this);
        }

        cvar & operator=(const std::string & value) {
            bool changed = this->string != value;
            this->string = value;
            try {
                this->value = std::stof(value);
            } catch (std::invalid_argument) {
                this->value = 0;
            }
            if (this->server && changed) {
                SV_BroadcastPrintf ("\"%s\" changed to \"%s\"\n", this->name.c_str(), this->string.c_str());
            }
            return *this;
        }
        cvar & operator=(float value) {
            return *this = std::to_string(value);
        }

        cvar * next() const;

        const std::string name;
        std::string string;
        bool archive;
        bool server;
        float value;

    private:
        static void install(cvar * v);
        static void uninstall(cvar * v);
    };
}

#endif

