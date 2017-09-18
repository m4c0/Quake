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

#include "quake/legacy.hpp"
#include "quake/wad.hpp"

#include <ios>

#if __BYTE_ORDER__ == __BIG_ENDIAN__
#error "SwapPic is required (as well as changing endianess of other fields)"
#endif

struct wadinfo {
    uint32_t identification; // WAD2
    int32_t numlumps;
    int32_t infotableofs;
};
struct lumpinfo {
    int32_t filepos;
    int32_t disksize;
    int32_t size;
    char type;
    char compression;
    char pad1, pad2;
    char name[16]; // must be null terminated
};

quake::wad::file::file(const char * filename) {
    wadinfo * header = (wadinfo *)COM_LoadHunkFile(filename);
    if (!header) {
        throw std::ios_base::failure(std::string("Could not load ") + filename);
    }

    if (header->identification != '2DAW') {
        throw std::ios_base::failure(std::string("Wad file ") + filename + " doesn't have WAD2 id");
    }

    uint8_t * base = (uint8_t *)header;
    lumpinfo * lump = (lumpinfo *)(header->infotableofs + base);

    for (int i = 0; i < header->numlumps; i++, lump++) {
        std::string name;
        std::transform(lump->name, lump->name + 16, std::back_inserter(name), ::toupper);

        auto & data = this->lumps[name.c_str()]; // Takes extra \0 away
        std::copy_n(base + lump->filepos, lump->size, std::back_inserter(data));
    }
}

std::vector<uint8_t> & quake::wad::file::at(const std::string & name) {
    std::string ucname;
    std::transform(name.begin(), name.end(), std::back_inserter(ucname), ::toupper);
    return this->lumps.at(ucname);
}

