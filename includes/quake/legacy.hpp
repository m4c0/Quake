#ifndef QUAKE_LEGACY_HPP
#define QUAKE_LEGACY_HPP

// One day, all "legacy" will be migrated and this file will be deleted
// This header exists to ease the pace of migrating "non-terminal" functions

extern char com_gamedir[128];
extern int glwidth, glheight, glx, gly;

void Con_Printf (char *fmt, ...);
int	Sys_FileTime (const char *path);

#endif

