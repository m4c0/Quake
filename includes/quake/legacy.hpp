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
void Con_Printf (const char *fmt, ...);
void SV_BroadcastPrintf (const char *fmt, ...);
int	Sys_FileTime (const char *path);

#endif

