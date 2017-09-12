#ifndef QUAKE_CMD_HPP
#define QUAKE_CMD_HPP

#include "quake/common.hpp"
#include "quake/legacy.hpp"

#include <functional>
#include <numeric>
#include <string>

namespace quake {
    namespace cmd {
        class base {
        public:
            base() {
            }
            virtual ~base() {
            }

            virtual void execute(const quake::common::argv & args) = 0;
        };

        class compat : public base {
        public:
            typedef void (*callback)(const quake::common::argv &);

            compat(callback fn) : base(), fn(fn) {
            }

            void execute(const quake::common::argv & args) override {
                fn(args);
            }

        private:
            callback fn;
        };

        class alias : public base {
        public:
            alias(const quake::common::argv & argv) : base() {
                name = argv[0];
                set_value(argv);
            }

            void execute(const quake::common::argv & args) override {
                Cbuf_InsertText(value);
            }

            std::string description() {
                return name + " = " + value;
            }

            void set_value(const quake::common::argv & argv) {
                if (argv.size() == 1) {
                    value = "";
                } else {
                    value = std::accumulate(argv.begin() + 2, argv.end(), argv[1], [](auto a, auto b) { return a + " " + b; });
                }
            }

        private:
            std::string name;
            std::string value;
        };

        base * by_name(const std::string & name);
        const char * complete(const std::string &);
        void find_by_type(const std::type_info & type, std::function<void(base *)> callback);

        template<class T>
        void find_by_type(std::function<void(T *)> callback) {
            find_by_type(typeid(T), [&](base * cmd) { callback(dynamic_cast<T *>(cmd)); });
        }

        void install(const std::string & name, base * v);
    }
}

#endif

