/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// cmd.c -- Quake script command processing module

#include "quakedef.h"

#include "quake/cmd.hpp"
#include "quake/common.hpp"

#include <map>
#include <numeric>

void Cmd_ForwardToServer (const quake::common::argv & argv);

qboolean	cmd_wait;

//=============================================================================

/*
============
Cmd_Wait_f

Causes execution of the remainder of the command buffer to be delayed until
next frame.  This allows commands like:
bind g "impulse 5 ; +attack ; wait ; -attack ; impulse 2"
============
*/
void Cmd_Wait_f (const quake::common::argv & argv)
{
	cmd_wait = true;
}

/*
=============================================================================

						COMMAND BUFFER

=============================================================================
*/

std::string cmd_text;

/*
============
Cbuf_Init
============
*/
void Cbuf_Init (void) {
}


/*
============
Cbuf_AddText

Adds command text at the end of the buffer
============
*/
void Cbuf_AddText (const std::string & text) {
    cmd_text.append(text);
}


/*
============
Cbuf_InsertText

Adds command text immediately after the current command
Adds a \n to the text
FIXME: actually change the command buffer to do less copying
============
*/
void Cbuf_InsertText (const std::string & text) {
    cmd_text.insert(0, text);
}

/*
============
Cbuf_Execute
============
*/
void Cbuf_Execute (void)
{
	int		i;
	char	*text;
	char	line[1024];
	int		quotes;
	
	while (cmd_text.size())
	{
// find a \n or ; line break
		text = (char *)cmd_text.c_str();

		quotes = 0;
		for (i=0 ; i< cmd_text.size(); i++)
		{
			if (text[i] == '"')
				quotes++;
			if ( !(quotes&1) &&  text[i] == ';')
				break;	// don't break if inside a quoted string
			if (text[i] == '\n')
				break;
		}
			
				
		memcpy (line, text, i);
		line[i] = 0;
		
// delete the text from the command buffer and move remaining commands down
// this is necessary because commands (exec, alias) can insert data at the
// beginning of the text buffer

		if (i == cmd_text.size()) {
			cmd_text.clear();
        } else {
            cmd_text = cmd_text.substr(i + 1);
		}

// execute the command line
		Cmd_ExecuteString (line, src_command);
		
		if (cmd_wait)
		{	// skip out while text still remains in buffer, leaving it
			// for next frame
			cmd_wait = false;
			break;
		}
	}
}

/*
==============================================================================

						SCRIPT COMMANDS

==============================================================================
*/

/*
===============
Cmd_StuffCmds_f

Adds command line parameters as script statements
Commands lead with a +, and continue until a - or another +
quake +prog jctest.qp +cmd amlev1
quake -nosound +cmd amlev1
===============
*/
void Cmd_StuffCmds_f (const quake::common::argv & args)
{
    auto & argv = quake::common::argv::current;
		
	if (args.size() != 0) {
		Con_Printf ("stuffcmds : execute command line parameters\n");
		return;
	}

// build the combined string to parse from
    if (argv->size() == 0) return;
    std::string text = std::accumulate(
            std::next(argv->begin()),
            argv->end(),
            *argv->begin(),
            [](auto & a, auto & b) { return a + ' ' + b; });
    if (text == "") return;

// pull out the commands
    std::string build;
    for (auto it = text.begin(); it != text.end();) {
        if (*it != '+') {
            it++;
            continue;
        }

        auto cmd_s = ++it;
        for (; it != text.end(); it++) {
            if (*it == '+') break;
            if (*it == '-') break;
        }

        build.append(cmd_s, it);
        build.append("\n");
    }
	
	if (build.size() > 0) {
		Cbuf_InsertText (build);
    }
}


/*
===============
Cmd_Exec_f
===============
*/
void Cmd_Exec_f (const quake::common::argv & argv)
{
	char	*f;
	int		mark;

	if (argv.size() != 1) {
		Con_Printf ("exec <filename> : execute a script file\n");
		return;
	}

	mark = Hunk_LowMark ();
	f = (char *)COM_LoadHunkFile (argv[0].c_str());
	if (!f)
	{
		Con_Printf ("couldn't exec %s\n", argv[0].c_str());
		return;
	}
	Con_Printf ("execing %s\n", argv[0].c_str());
	
	Cbuf_InsertText (f);
	Hunk_FreeToLowMark (mark);
}


/*
===============
Cmd_Echo_f

Just prints the rest of the line to the console
===============
*/
void Cmd_Echo_f (const quake::common::argv & argv)
{
    for (auto & s : argv) {
        Con_Printf("%s ", s.c_str());
    }
	Con_Printf ("\n");
}

/*
===============
Cmd_Alias_f

Creates a new command that executes a command string (possibly ; seperated)
===============
*/

void Cmd_Alias_f (const quake::common::argv & argv)
{
	char		cmd[1024];
	int			i, c;

	if (argv.size() == 0) {
		Con_Printf ("Current alias commands:\n");
        quake::cmd::find_by_type<quake::cmd::alias>([](auto a) {
			Con_Printf("%s\n", a->description().c_str());
        });
		return;
	}

    std::string name = argv[0];
    try {
        auto cmd = quake::cmd::by_name(name);
        auto alias = dynamic_cast<quake::cmd::alias *>(cmd);
        if (alias) {
            alias->set_value(argv);
            return;
        }
    } catch (...) {
    }
    quake::cmd::install(name, new quake::cmd::alias(argv));
}

/*
=============================================================================

					COMMAND EXECUTION

=============================================================================
*/

cmd_source_t	cmd_source;


/*
============
Cmd_Init
============
*/
void Cmd_Init (void)
{
//
// register our commands
//
	Cmd_AddCommand ("stuffcmds",Cmd_StuffCmds_f);
	Cmd_AddCommand ("exec",Cmd_Exec_f);
	Cmd_AddCommand ("echo",Cmd_Echo_f);
	Cmd_AddCommand ("alias",Cmd_Alias_f);
	Cmd_AddCommand ("cmd", Cmd_ForwardToServer);
	Cmd_AddCommand ("wait", Cmd_Wait_f);
}

/*
============
Cmd_AddCommand
============
*/
void	Cmd_AddCommand (const char *cmd_name, xcommand_t function)
{
	if (host_initialized)	// because hunk allocation would get stomped
		Sys_Error ("Cmd_AddCommand after host_initialized");
		
// fail if the command is a variable name
    try {
	    quake::cvar::by_name(cmd_name);
		Con_Printf ("Cmd_AddCommand: %s already defined as a var\n", cmd_name);
		return;
    } catch (...) {
	}

    quake::cmd::install(cmd_name, new quake::cmd::compat(function));
}

/*
============
Cmd_ExecuteString

A complete command line has been parsed, so try to execute it
FIXME: lookupnoadd the token to speed search?
============
*/
void	Cmd_ExecuteString (const char *text, cmd_source_t src)
{	
	cmd_source = src;

    quake::common::argv args { text };
			
// execute the command line
	if (args.cmd == "")
		return;		// no tokens

// check functions/aliases
    try {
        quake::cmd::by_name(args.cmd)->execute(args);
        return;
    } catch (...) {
    }

// check cvars
	//if (!Cvar_Command (args.cmd.c_str(), args))
    try {
        auto v = quake::cvar::by_name(args.cmd);
        if (args.size() == 0) {
            Con_Printf ("\"%s\" is \"%s\"\n", v->name.c_str(), v->to_cstr());
        } else {
            *v = args[0];
        }
    } catch (...) {
		Con_Printf ("Unknown command \"%s\"\n", args.cmd.c_str());
    }
	
}


/*
===================
Cmd_ForwardToServer

Sends the entire command line over to the server
===================
*/
void Cmd_ForwardToServer (const quake::common::argv & args)
{
	if (cls.state != ca_connected)
	{
		Con_Printf ("Can't \"%s\", not connected\n", args.cmd.c_str());
		return;
	}
	
	if (cls.demoplayback)
		return;		// not really connected

	MSG_WriteByte (&cls.message, clc_stringcmd);
	if (Q_strcasecmp(args.cmd.c_str(), "cmd") != 0)
	{
		SZ_Print (&cls.message, args.cmd.c_str());
		SZ_Print (&cls.message, " ");
	}
	if (args.size() > 0) {
		SZ_Print (&cls.message, args.all.c_str());
    } else {
		SZ_Print (&cls.message, "\n");
    }
}

