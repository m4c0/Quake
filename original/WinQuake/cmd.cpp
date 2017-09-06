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

#include "quake/common.hpp"

#include <map>
#include <numeric>

void Cmd_ForwardToServer (const quake::common::argv & argv);

static std::map<std::string, std::string> _aliases;

int trashtest;
int *trashspot;

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

	if (Cmd_Argc () != 2)
	{
		Con_Printf ("exec <filename> : execute a script file\n");
		return;
	}

	mark = Hunk_LowMark ();
	f = (char *)COM_LoadHunkFile (Cmd_Argv(1));
	if (!f)
	{
		Con_Printf ("couldn't exec %s\n",Cmd_Argv(1));
		return;
	}
	Con_Printf ("execing %s\n",Cmd_Argv(1));
	
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
	int		i;
	
	for (i=1 ; i<Cmd_Argc() ; i++)
		Con_Printf ("%s ",Cmd_Argv(i));
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

	if (Cmd_Argc() == 1)
	{
		Con_Printf ("Current alias commands:\n");
        for (auto & kv : _aliases) {
			Con_Printf("%s : %s\n", kv.first.c_str(), kv.second.c_str());
        }
		return;
	}

    std::string name = Cmd_Argv(1);

    std::string args;
    for (int i = 2; i < Cmd_Argc(); i++) {
        args += Cmd_Argv(i);
        args += " ";
    }
    _aliases[name] = args;
}

/*
=============================================================================

					COMMAND EXECUTION

=============================================================================
*/

typedef struct cmd_function_s
{
	struct cmd_function_s	*next;
	const char					*name;
	xcommand_t				function;
} cmd_function_t;


#define	MAX_ARGS		80

static	int			cmd_argc;
static	char		*cmd_argv[MAX_ARGS];
static	const char		*cmd_null_string = "";
static	const char		*cmd_args = NULL;

cmd_source_t	cmd_source;


static	cmd_function_t	*cmd_functions;		// possible commands to execute

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
Cmd_Argc
============
*/
int		Cmd_Argc (void)
{
	return cmd_argc;
}

/*
============
Cmd_Argv
============
*/
const char	*Cmd_Argv (int arg)
{
	if ( (unsigned)arg >= cmd_argc )
		return cmd_null_string;
	return cmd_argv[arg];	
}

/*
============
Cmd_Args
============
*/
const char		*Cmd_Args (void)
{
	return cmd_args;
}


/*
============
Cmd_TokenizeString

Parses the given string into command line tokens.
============
*/
void Cmd_TokenizeString (const char *text)
{
	int		i;
	
// clear the args from the last string
	for (i=0 ; i<cmd_argc ; i++)
		Z_Free (cmd_argv[i]);
		
	cmd_argc = 0;
	cmd_args = NULL;
	
	while (1)
	{
// skip whitespace up to a /n
		while (*text && *text <= ' ' && *text != '\n')
		{
			text++;
		}
		
		if (*text == '\n')
		{	// a newline seperates commands in the buffer
			text++;
			break;
		}

		if (!*text)
			return;
	
		if (cmd_argc == 1)
			 cmd_args = text;
			
		text = COM_Parse (text);
		if (!text)
			return;

		if (cmd_argc < MAX_ARGS)
		{
			cmd_argv[cmd_argc] = (char *)Z_Malloc (Q_strlen(com_token)+1);
			Q_strcpy (cmd_argv[cmd_argc], com_token);
			cmd_argc++;
		}
	}
	
}


/*
============
Cmd_AddCommand
============
*/
void	Cmd_AddCommand (const char *cmd_name, xcommand_t function)
{
	cmd_function_t	*cmd;
	
	if (host_initialized)	// because hunk allocation would get stomped
		Sys_Error ("Cmd_AddCommand after host_initialized");
		
// fail if the command is a variable name
	if (Cvar_VariableString(cmd_name)[0])
	{
		Con_Printf ("Cmd_AddCommand: %s already defined as a var\n", cmd_name);
		return;
	}
	
// fail if the command already exists
	for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
	{
		if (!Q_strcmp (cmd_name, cmd->name))
		{
			Con_Printf ("Cmd_AddCommand: %s already defined\n", cmd_name);
			return;
		}
	}

	cmd = (cmd_function_t *)Hunk_Alloc (sizeof(cmd_function_t));
	cmd->name = cmd_name;
	cmd->function = function;
	cmd->next = cmd_functions;
	cmd_functions = cmd;
}

/*
============
Cmd_Exists
============
*/
qboolean	Cmd_Exists (const char *cmd_name)
{
	cmd_function_t	*cmd;

	for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
	{
		if (!Q_strcmp (cmd_name,cmd->name))
			return true;
	}

	return false;
}



/*
============
Cmd_CompleteCommand
============
*/
const char *Cmd_CompleteCommand (const char *partial)
{
	cmd_function_t	*cmd;
	int				len;
	
	len = Q_strlen(partial);
	
	if (!len)
		return NULL;
		
// check functions
	for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
		if (!Q_strncmp (partial,cmd->name, len))
			return cmd->name;

	return NULL;
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
	cmd_function_t	*cmd;

	cmd_source = src;
	Cmd_TokenizeString (text);
			
// execute the command line
	if (!Cmd_Argc())
		return;		// no tokens

    quake::common::argv args(cmd_argc, cmd_argv);
    args.all = cmd_args;

// check functions
	for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
	{
		if (!Q_strcasecmp (cmd_argv[0],cmd->name))
		{
			cmd->function (args);
			return;
		}
	}

// check alias
    for (auto & kv : _aliases) {
        if (kv.first == cmd_argv[0]) {
			Cbuf_InsertText(kv.second);
			return;
		}
	}
	
// check cvars
	if (!Cvar_Command (cmd_argv[0], args))
		Con_Printf ("Unknown command \"%s\"\n", cmd_argv[0]);
	
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
		Con_Printf ("Can't \"%s\", not connected\n", Cmd_Argv(0));
		return;
	}
	
	if (cls.demoplayback)
		return;		// not really connected

	MSG_WriteByte (&cls.message, clc_stringcmd);
	if (Q_strcasecmp(Cmd_Argv(0), "cmd") != 0)
	{
		SZ_Print (&cls.message, Cmd_Argv(0));
		SZ_Print (&cls.message, " ");
	}
	if (Cmd_Argc() > 1)
		SZ_Print (&cls.message, Cmd_Args());
	else
		SZ_Print (&cls.message, "\n");
}

