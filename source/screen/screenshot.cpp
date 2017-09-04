#include "quake/common.hpp"
#include "quake/gl.hpp"
#include "quake/legacy.hpp"
#include "quake/screen.hpp"

// TODO: move this to a "impl"-like file
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <string>
#include <vector>

static void _take_shot_named(const std::string & name) {
    std::vector<uint8_t> data(glwidth * glheight * 3);
    glReadPixels(glx, gly, glwidth, glheight, GL_RGB, GL_UNSIGNED_BYTE, data.data());

    // XXX: Is there a better way to flip glReadPixels' result?
    for (int y = 0; y < glheight / 2; y++) {
        auto p1s = data.begin() + y * glwidth * 3;
        auto p1e = p1s + glwidth * 3 - 1;
        auto p2s = data.begin() + (glheight - y - 1) * glwidth * 3;
        std::swap_ranges(p1s, p1e, p2s);
    }

    stbi_write_png(name.c_str(), glwidth, glheight, 3, data.data(), glwidth * 3);
    Con_Printf ("Wrote %s\n", name.c_str());
}

void quake::screen::screenshot(const quake::common::argv & argv) {
    std::string name { "/quake00.png" };
    for (int i = 0; i <= 99; i++) {
        name[6] = '0' + (i / 10);
        name[7] = '0' + (i % 10);

        std::string fullname = com_gamedir + name;
        if (Sys_FileTime(fullname.c_str()) == -1) {
            _take_shot_named(fullname);
            return;
        }
    }
    Con_Printf("SCR_ScreenShot_f: Couldn't create a PNG file\n"); 
}

