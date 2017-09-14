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

#ifndef QUAKE_LEGACY_HPP
#define QUAKE_LEGACY_HPP

#include <string>

// One day, all "legacy" will be migrated and this file will be deleted
// This header exists to ease the pace of migrating "non-terminal" functions

extern char com_token[1024];
extern char com_gamedir[128];
extern int glwidth, glheight, glx, gly;

void Cbuf_InsertText (const std::string & text);
const char *COM_Parse (const char *data);
uint8_t * COM_LoadHunkFile (const char *path);
void Con_Printf (const char *fmt, ...);
void SV_BroadcastPrintf (const char *fmt, ...);
int	Sys_FileTime (const char *path);

#endif

