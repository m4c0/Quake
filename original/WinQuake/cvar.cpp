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
// cvar.c -- dynamic variable tracking

#include "quakedef.h"

cvar_t	*cvar_vars;
const char	*cvar_null_string = "";

/*
============
Cvar_FindVar
============
*/
cvar_t * Cvar_FindVar (const char *var_name) {
    for (cvar_t * var = cvar_vars; var; var = var->next) {
        if (var->name == var_name) {
            return var;
        }
    }
	return nullptr;
}

/*
============
Cvar_VariableValue
============
*/
float Cvar_VariableValue (const char *var_name) {
	cvar_t * var = Cvar_FindVar(var_name);
    return var ? std::stof(var->string) : 0;
}


/*
============
Cvar_VariableString
============
*/
const char *Cvar_VariableString (const char *var_name) {
	cvar_t * var = Cvar_FindVar(var_name);
    return var ? var->string.c_str() : cvar_null_string;
}


/*
============
Cvar_CompleteVariable
============
*/
const char *Cvar_CompleteVariable (const char *partial)
{
	int			len;
	
	len = Q_strlen(partial);
	
	if (!len)
		return NULL;
		
// check functions
    for (cvar_t * var = cvar_vars; var; var = var->next) {
        if (var->name.compare(0, len, partial) == 0) {
            return var->name.c_str();
        }
    }

	return NULL;
}


/*
============
Cvar_Set
============
*/
void Cvar_Set (const char *var_name, const char *value)
{
	cvar_t	*var;
	qboolean changed;
	
	var = Cvar_FindVar (var_name);
	if (!var)
	{	// there is an error in C code if this happens
		Con_Printf ("Cvar_Set: variable %s not found\n", var_name);
		return;
	}

	changed = var->string != value;
	
    var->string = value;
    try {
        var->value = std::stof(var->string);
    } catch (std::invalid_argument) {
        var->value = 0;
    }
	if (var->server && changed)
	{
		if (sv.active)
			SV_BroadcastPrintf ("\"%s\" changed to \"%s\"\n", var->name.c_str(), var->string.c_str());
	}
}

/*
============
Cvar_SetValue
============
*/
void Cvar_SetValue (const char *var_name, float value)
{
	char	val[32];
	
	sprintf (val, "%f",value);
	Cvar_Set (var_name, val);
}


/*
============
Cvar_RegisterVariable

Adds a freestanding variable to the variable list.
============
*/
void Cvar_RegisterVariable (cvar_t *variable)
{
// first check to see if it has allready been defined
	if (Cvar_FindVar (variable->name.c_str()))
	{
		Con_Printf ("Can't register variable %s, allready defined\n", variable->name.c_str());
		return;
	}
	
// check for overlap with a command
	if (Cmd_Exists (variable->name.c_str()))
	{
		Con_Printf ("Cvar_RegisterVariable: %s is a command\n", variable->name.c_str());
		return;
	}

    try {
        variable->value = std::stof(variable->string);
    } catch (std::invalid_argument) {
        variable->value = 0;
    }
	
// link the variable in
	variable->next = cvar_vars;
	cvar_vars = variable;
}

/*
============
Cvar_Command

Handles variable inspection and changing from the console
============
*/
qboolean Cvar_Command (std::string cmd, const quake::common::argv & args) {
	cvar_t			*v;

// check variables
	v = Cvar_FindVar (cmd.c_str());
	if (!v)
		return false;
		
// perform a variable print or set
	if (args.size() == 0) {
		Con_Printf ("\"%s\" is \"%s\"\n", v->name.c_str(), v->string.c_str());
		return true;
	}

	Cvar_Set (v->name.c_str(), args[0].c_str());
	return true;
}


/*
============
Cvar_WriteVariables

Writes lines containing "set variable value" for all variables
with the archive flag set to true.
============
*/
void Cvar_WriteVariables (FILE *f)
{
	cvar_t	*var;
	
	for (var = cvar_vars ; var ; var = var->next)
		if (var->archive)
			fprintf (f, "%s \"%s\"\n", var->name.c_str(), var->string.c_str());
}

