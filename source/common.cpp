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

#include "quake/common.hpp"

std::unique_ptr<const quake::common::argv> quake::common::argv::current { new quake::common::argv(0, nullptr) };
