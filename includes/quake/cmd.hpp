#ifndef QUAKE_CMD_HPP
#define QUAKE_CMD_HPP

#include "quake/common.hpp"

#include <string>

namespace quake {
    namespace cmd {
        class base {
        public:
            base() {
            }
            virtual ~base() {
            }

            virtual void execute(const quake::common::argv & args) const = 0;
        };

        class compat : public base {
        public:
            typedef void (*callback)(const quake::common::argv &);

            compat(callback fn) : fn(fn) {
            }

            void execute(const quake::common::argv & args) const override {
                fn(args);
            }

        private:
            callback fn;
        };

        base * by_name(const std::string & name);
        const char * complete(const std::string &);

        void install(const std::string & name, base * v);
    }
}

#endif

