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

#ifndef QUAKE_TEXTURE_HPP
#define QUAKE_TEXTURE_HPP

#include "quake/gl.hpp"

namespace quake {
    class texture {
    public:
        texture() {
            glGenTextures(1, &texnum);
        }

        void bind() {
            glBindTexture(GL_TEXTURE_2D, texnum);
        }

        virtual void set_filters(GLenum min, GLenum max) {
        }

    private:
        GLuint texnum;
    };

    class mipmap_texture : public texture {
    public:
        // Only textures created with "mipmap support" can change filters,
        // even though the user is able to use "GL_LINEAR" and "GL_NEAREST"
        // as filters.
        void set_filters(GLenum min, GLenum max) override {
            this->bind();
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, max);
        }
    };

    class sprite {
    public:
        sprite() : texnum(nullptr), sl(0), tl(0), sh(0), th(0), width(0), height(0) {
        }
        // TODO: receive the texture as param
        sprite(int w, int h) : texnum(nullptr), sl(0), tl(0), sh(1), th(1), width(w), height(h) {
        }

        void draw(int x, int y) {
            this->texnum->bind();
            glColor4f(1, 1, 1, 1);

            glBegin(GL_QUADS);
            glTexCoord2f(this->sl, this->tl); glVertex2f(x, y);
            glTexCoord2f(this->sh, this->tl); glVertex2f(x + this->width, y);
            glTexCoord2f(this->sh, this->th); glVertex2f(x + this->width, y + this->height);
            glTexCoord2f(this->sl, this->th); glVertex2f(x, y + this->height);
            glEnd();
        }
        void draw(int x, int y, float alpha) {
            this->texnum->bind();
            glDisable(GL_ALPHA_TEST);
            glEnable(GL_BLEND);
            glColor4f(1, 1, 1, alpha);

            glBegin(GL_QUADS);
            glTexCoord2f(this->sl, this->tl); glVertex2f(x, y);
            glTexCoord2f(this->sh, this->tl); glVertex2f(x + this->width, y);
            glTexCoord2f(this->sh, this->th); glVertex2f(x + this->width, y + this->height);
            glTexCoord2f(this->sl, this->th); glVertex2f(x, y + this->height);
            glEnd();

            glColor4f(1, 1, 1, 1);
            glEnable(GL_ALPHA_TEST);
            glDisable(GL_BLEND);
        }

        std::shared_ptr<texture> texnum;
        float sl, tl, sh, th;
        int width, height;
    };
}

#endif


