#include "quake/common.hpp"

std::unique_ptr<quake::common::argv> quake::common::argv::current { new quake::common::argv(0, nullptr) };
