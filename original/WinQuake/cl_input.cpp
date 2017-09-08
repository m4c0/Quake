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
// cl.input.c  -- builds an intended movement command to send to the server

// Quake is a trademark of Id Software, Inc., (c) 1996 Id Software, Inc. All
// rights reserved.

#include "quakedef.h"

/*
===============================================================================

KEY BUTTONS

Continuous button event tracking is complicated by the fact that two different
input sources (say, mouse button 1 and the control key) can both press the
same button, but the button should only be released when both of the
pressing key have been released.

When a key event issues a button command (+forward, +attack, etc), it appends
its key number as a parameter to the command so it can be matched up with
the release.

state bit 0 is the current state of the key
state bit 1 is edge triggered on the up to down transition
state bit 2 is edge triggered on the down to up transition

===============================================================================
*/

using args = quake::common::argv;

kbutton_t	in_mlook, in_klook;
kbutton_t	in_left, in_right, in_forward, in_back;
kbutton_t	in_lookup, in_lookdown, in_moveleft, in_moveright;
kbutton_t	in_strafe, in_speed, in_use, in_jump, in_attack;
kbutton_t	in_up, in_down;

int			in_impulse;


static void KeyDown(kbutton_t * b, const args & args) {
	int k = args.stoi(0, -1); // k==-1 -> typed manually at the console for continuous down

	if (k == b->down[0] || k == b->down[1])
		return;		// repeating key
	
	if (!b->down[0])
		b->down[0] = k;
	else if (!b->down[1])
		b->down[1] = k;
	else
	{
		Con_Printf ("Three keys down for a button!\n");
		return;
	}
	
	if (b->state & 1)
		return;		// still down
	b->state |= 1 + 2;	// down + impulse down
}

static void KeyUp(kbutton_t * b, const args & args) {
	int k = args.stoi(0, -1);
    if (k == -1) { // typed manually at the console, assume for unsticking, so clear all
		b->down[0] = b->down[1] = 0;
		b->state = 4;	// impulse up
		return;
	}

	if (b->down[0] == k)
		b->down[0] = 0;
	else if (b->down[1] == k)
		b->down[1] = 0;
	else
		return;		// key up without coresponding down (menu pass through)
	if (b->down[0] || b->down[1])
		return;		// some other key is still holding it down

	if (!(b->state & 1))
		return;		// still up (this should not happen)
	b->state &= ~1;		// now up
	b->state |= 4; 		// impulse up
}

void IN_KLookDown (const args & args) { KeyDown(&in_klook, args); }
void IN_KLookUp   (const args & args) { KeyUp  (&in_klook, args); }
void IN_MLookDown (const args & args) { KeyDown(&in_mlook, args); }
void IN_MLookUp   (const args & args) {
    KeyUp(&in_mlook, args);
    if ( !(in_mlook.state&1) &&  lookspring.value)
        V_StartPitchDrift({});
}
void IN_UpDown       (const args & args) { KeyDown(&in_up,        args); }
void IN_UpUp         (const args & args) { KeyUp  (&in_up,        args); }
void IN_DownDown     (const args & args) { KeyDown(&in_down,      args); }
void IN_DownUp       (const args & args) { KeyUp  (&in_down,      args); }
void IN_LeftDown     (const args & args) { KeyDown(&in_left,      args); }
void IN_LeftUp       (const args & args) { KeyUp  (&in_left,      args); }
void IN_RightDown    (const args & args) { KeyDown(&in_right,     args); }
void IN_RightUp      (const args & args) { KeyUp  (&in_right,     args); }
void IN_ForwardDown  (const args & args) { KeyDown(&in_forward,   args); }
void IN_ForwardUp    (const args & args) { KeyUp  (&in_forward,   args); }
void IN_BackDown     (const args & args) { KeyDown(&in_back,      args); }
void IN_BackUp       (const args & args) { KeyUp  (&in_back,      args); }
void IN_LookupDown   (const args & args) { KeyDown(&in_lookup,    args); }
void IN_LookupUp     (const args & args) { KeyUp  (&in_lookup,    args); }
void IN_LookdownDown (const args & args) { KeyDown(&in_lookdown,  args); }
void IN_LookdownUp   (const args & args) { KeyUp  (&in_lookdown,  args); }
void IN_MoveleftDown (const args & args) { KeyDown(&in_moveleft,  args); }
void IN_MoveleftUp   (const args & args) { KeyUp  (&in_moveleft,  args); }
void IN_MoverightDown(const args & args) { KeyDown(&in_moveright, args); }
void IN_MoverightUp  (const args & args) { KeyUp  (&in_moveright, args); }

void IN_SpeedDown    (const args & args) { KeyDown(&in_speed,  args); }
void IN_SpeedUp      (const args & args) { KeyUp  (&in_speed,  args); }
void IN_StrafeDown   (const args & args) { KeyDown(&in_strafe, args); }
void IN_StrafeUp     (const args & args) { KeyUp  (&in_strafe, args); }

void IN_AttackDown   (const args & args) { KeyDown(&in_attack, args); }
void IN_AttackUp     (const args & args) { KeyUp  (&in_attack, args); }

void IN_UseDown      (const args & args) { KeyDown(&in_use,  args); }
void IN_UseUp        (const args & args) { KeyUp  (&in_use,  args); }
void IN_JumpDown     (const args & args) { KeyDown(&in_jump, args); }
void IN_JumpUp       (const args & args) { KeyUp  (&in_jump, args); }

void IN_Impulse      (const args & args) { in_impulse = std::stoi(args[0]); }

/*
===============
CL_KeyState

Returns 0.25 if a key was pressed and released during the frame,
0.5 if it was pressed and held
0 if held then released, and
1.0 if held for the entire time
===============
*/
float CL_KeyState (kbutton_t *key)
{
	float		val;
	qboolean	impulsedown, impulseup, down;
	
	impulsedown = key->state & 2;
	impulseup = key->state & 4;
	down = key->state & 1;
	val = 0;
	
	if (impulsedown && !impulseup) {
		if (down) {
			val = 0.5;	// pressed and held this frame
		} else {
			val = 0;	//	I_Error ();
        }
    }
	if (impulseup && !impulsedown) {
		if (down) {
			val = 0;	//	I_Error ();
        } else {
			val = 0;	// released this frame
        }
    }
	if (!impulsedown && !impulseup) {
		if (down) {
			val = 1.0;	// held the entire frame
        } else {
			val = 0;	// up the entire frame
        }
    }
	if (impulsedown && impulseup) {
		if (down) {
			val = 0.75;	// released and re-pressed this frame
        } else {
			val = 0.25;	// pressed and released this frame
        }
    }

	key->state &= 1;		// clear impulses
	
	return val;
}




//==========================================================================

quake::cvar::normal cl_upspeed = {"cl_upspeed","200"};
quake::cvar::normal cl_forwardspeed = {"cl_forwardspeed","200", true};
quake::cvar::normal cl_backspeed = {"cl_backspeed","200", true};
quake::cvar::normal cl_sidespeed = {"cl_sidespeed","350"};

quake::cvar::normal cl_movespeedkey = {"cl_movespeedkey","2.0"};

quake::cvar::normal cl_yawspeed = {"cl_yawspeed","140"};
quake::cvar::normal cl_pitchspeed = {"cl_pitchspeed","150"};

quake::cvar::normal cl_anglespeedkey = {"cl_anglespeedkey","1.5"};


/*
================
CL_AdjustAngles

Moves the local angle positions
================
*/
void CL_AdjustAngles (void)
{
	float	speed;
	float	up, down;
	
	if (in_speed.state & 1)
		speed = host_frametime * cl_anglespeedkey.value;
	else
		speed = host_frametime;

	if (!(in_strafe.state & 1))
	{
		cl.viewangles[YAW] -= speed*cl_yawspeed.value*CL_KeyState (&in_right);
		cl.viewangles[YAW] += speed*cl_yawspeed.value*CL_KeyState (&in_left);
		cl.viewangles[YAW] = anglemod(cl.viewangles[YAW]);
	}
	if (in_klook.state & 1)
	{
		V_StopPitchDrift ();
		cl.viewangles[PITCH] -= speed*cl_pitchspeed.value * CL_KeyState (&in_forward);
		cl.viewangles[PITCH] += speed*cl_pitchspeed.value * CL_KeyState (&in_back);
	}
	
	up = CL_KeyState (&in_lookup);
	down = CL_KeyState(&in_lookdown);
	
	cl.viewangles[PITCH] -= speed*cl_pitchspeed.value * up;
	cl.viewangles[PITCH] += speed*cl_pitchspeed.value * down;

	if (up || down)
		V_StopPitchDrift ();
		
	if (cl.viewangles[PITCH] > 80)
		cl.viewangles[PITCH] = 80;
	if (cl.viewangles[PITCH] < -70)
		cl.viewangles[PITCH] = -70;

	if (cl.viewangles[ROLL] > 50)
		cl.viewangles[ROLL] = 50;
	if (cl.viewangles[ROLL] < -50)
		cl.viewangles[ROLL] = -50;
		
}

/*
================
CL_BaseMove

Send the intended movement message to the server
================
*/
void CL_BaseMove (usercmd_t *cmd)
{	
	if (cls.signon != SIGNONS)
		return;
			
	CL_AdjustAngles ();
	
	Q_memset (cmd, 0, sizeof(*cmd));
	
	if (in_strafe.state & 1)
	{
		cmd->sidemove += cl_sidespeed.value * CL_KeyState (&in_right);
		cmd->sidemove -= cl_sidespeed.value * CL_KeyState (&in_left);
	}

	cmd->sidemove += cl_sidespeed.value * CL_KeyState (&in_moveright);
	cmd->sidemove -= cl_sidespeed.value * CL_KeyState (&in_moveleft);

	cmd->upmove += cl_upspeed.value * CL_KeyState (&in_up);
	cmd->upmove -= cl_upspeed.value * CL_KeyState (&in_down);

	if (! (in_klook.state & 1) )
	{	
		cmd->forwardmove += cl_forwardspeed.value * CL_KeyState (&in_forward);
		cmd->forwardmove -= cl_backspeed.value * CL_KeyState (&in_back);
	}	

//
// adjust for speed key
//
	if (in_speed.state & 1)
	{
		cmd->forwardmove *= cl_movespeedkey.value;
		cmd->sidemove *= cl_movespeedkey.value;
		cmd->upmove *= cl_movespeedkey.value;
	}

#ifdef QUAKE2
	cmd->lightlevel = cl.light_level;
#endif
}



/*
==============
CL_SendMove
==============
*/
void CL_SendMove (usercmd_t *cmd)
{
	int		i;
	int		bits;
	sizebuf_t	buf;
	byte	data[128];
	
	buf.maxsize = 128;
	buf.cursize = 0;
	buf.data = data;
	
	cl.cmd = *cmd;

//
// send the movement message
//
    MSG_WriteByte (&buf, clc_move);

	MSG_WriteFloat (&buf, cl.mtime[0]);	// so server can get ping times

	for (i=0 ; i<3 ; i++)
		MSG_WriteAngle (&buf, cl.viewangles[i]);
	
    MSG_WriteShort (&buf, cmd->forwardmove);
    MSG_WriteShort (&buf, cmd->sidemove);
    MSG_WriteShort (&buf, cmd->upmove);

//
// send button bits
//
	bits = 0;
	
	if ( in_attack.state & 3 )
		bits |= 1;
	in_attack.state &= ~2;
	
	if (in_jump.state & 3)
		bits |= 2;
	in_jump.state &= ~2;
	
    MSG_WriteByte (&buf, bits);

    MSG_WriteByte (&buf, in_impulse);
	in_impulse = 0;

#ifdef QUAKE2
//
// light level
//
	MSG_WriteByte (&buf, cmd->lightlevel);
#endif

//
// deliver the message
//
	if (cls.demoplayback)
		return;

//
// allways dump the first two message, because it may contain leftover inputs
// from the last level
//
	if (++cl.movemessages <= 2)
		return;
	
	if (NET_SendUnreliableMessage (cls.netcon, &buf) == -1)
	{
		Con_Printf ("CL_SendMove: lost server connection\n");
		CL_Disconnect ();
	}
}

/*
============
CL_InitInput
============
*/
void CL_InitInput (void)
{
	Cmd_AddCommand ("+moveup",IN_UpDown);
	Cmd_AddCommand ("-moveup",IN_UpUp);
	Cmd_AddCommand ("+movedown",IN_DownDown);
	Cmd_AddCommand ("-movedown",IN_DownUp);
	Cmd_AddCommand ("+left",IN_LeftDown);
	Cmd_AddCommand ("-left",IN_LeftUp);
	Cmd_AddCommand ("+right",IN_RightDown);
	Cmd_AddCommand ("-right",IN_RightUp);
	Cmd_AddCommand ("+forward",IN_ForwardDown);
	Cmd_AddCommand ("-forward",IN_ForwardUp);
	Cmd_AddCommand ("+back",IN_BackDown);
	Cmd_AddCommand ("-back",IN_BackUp);
	Cmd_AddCommand ("+lookup", IN_LookupDown);
	Cmd_AddCommand ("-lookup", IN_LookupUp);
	Cmd_AddCommand ("+lookdown", IN_LookdownDown);
	Cmd_AddCommand ("-lookdown", IN_LookdownUp);
	Cmd_AddCommand ("+strafe", IN_StrafeDown);
	Cmd_AddCommand ("-strafe", IN_StrafeUp);
	Cmd_AddCommand ("+moveleft", IN_MoveleftDown);
	Cmd_AddCommand ("-moveleft", IN_MoveleftUp);
	Cmd_AddCommand ("+moveright", IN_MoverightDown);
	Cmd_AddCommand ("-moveright", IN_MoverightUp);
	Cmd_AddCommand ("+speed", IN_SpeedDown);
	Cmd_AddCommand ("-speed", IN_SpeedUp);
	Cmd_AddCommand ("+attack", IN_AttackDown);
	Cmd_AddCommand ("-attack", IN_AttackUp);
	Cmd_AddCommand ("+use", IN_UseDown);
	Cmd_AddCommand ("-use", IN_UseUp);
	Cmd_AddCommand ("+jump", IN_JumpDown);
	Cmd_AddCommand ("-jump", IN_JumpUp);
	Cmd_AddCommand ("impulse", IN_Impulse);
	Cmd_AddCommand ("+klook", IN_KLookDown);
	Cmd_AddCommand ("-klook", IN_KLookUp);
	Cmd_AddCommand ("+mlook", IN_MLookDown);
	Cmd_AddCommand ("-mlook", IN_MLookUp);

}

