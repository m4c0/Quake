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
#include <GLUT/glut.h>

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <signal.h>

#include <dlfcn.h>

#include "quakedef.h"

#include "quake/common.hpp"

#define GL_SHARED_TEXTURE_PALETTE_EXT   33275

#define WARP_WIDTH              320
#define WARP_HEIGHT             200

static auto & argv = quake::common::argv::current;

int glut_window = 0;

unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];
unsigned char d_15to8table[65536];

int num_shades=32;

int	d_con_indirect = 0;

int		svgalib_inited=0;
int		UseMouse = 1;
int		UseKeyboard = 1;

quake::cvar::normal vid_mode = {"vid_mode","5"};
quake::cvar::normal vid_redrawfull = {"vid_redrawfull","0"};
quake::cvar::persistent vid_waitforrefresh = {"vid_waitforrefresh","0"};
 
char	*framebuffer_ptr;

quake::cvar::normal mouse_button_commands[3] =
{
    {"mouse1","+attack"},
    {"mouse2","+strafe"},
    {"mouse3","+forward"},
};

int     mouse_buttons;
int     mouse_buttonstate[3];
int     mouse_oldbuttonstate[3];
float   mouse_x, mouse_y;
float	old_mouse_x, old_mouse_y;
int		mx, my;

quake::cvar::normal m_filter = {"m_filter","1"};

int scr_width, scr_height;

/*-----------------------------------------------------------------------*/

//int		texture_mode = GL_NEAREST;
//int		texture_mode = GL_NEAREST_MIPMAP_NEAREST;
//int		texture_mode = GL_NEAREST_MIPMAP_LINEAR;
int		texture_mode = GL_LINEAR;
//int		texture_mode = GL_LINEAR_MIPMAP_NEAREST;
//int		texture_mode = GL_LINEAR_MIPMAP_LINEAR;

int		texture_extension_number = 1;

float		gldepthmin, gldepthmax;

quake::cvar::normal gl_ztrick = {"gl_ztrick","1"};

const GLubyte *gl_vendor;
const GLubyte *gl_renderer;
const GLubyte *gl_version;
const GLubyte *gl_extensions;

void (*qgl3DfxSetPaletteEXT) (GLuint *);
void (*qglColorTableEXT) (int, int, int, int, int, const void *);

static float vid_gamma = 1.0;

qboolean is8bit = false;
qboolean isPermedia = false;
qboolean gl_mtexable = false;

/*-----------------------------------------------------------------------*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
}

void D_EndDirectRect (int x, int y, int width, int height)
{
}

void key_down_handler(unsigned char sc, int x, int y) {
    Key_Event(sc, 1);
}
void key_up_handler(unsigned char sc, int x, int y) {
    Key_Event(sc, 0);
}
void skey_handler(int key, int s) {
    switch (key) {
        case GLUT_KEY_F1:  Key_Event(K_F1,  s); break;
        case GLUT_KEY_F2:  Key_Event(K_F2,  s); break;
        case GLUT_KEY_F3:  Key_Event(K_F3,  s); break;
        case GLUT_KEY_F4:  Key_Event(K_F4,  s); break;
        case GLUT_KEY_F5:  Key_Event(K_F5,  s); break;
        case GLUT_KEY_F6:  Key_Event(K_F6,  s); break;
        case GLUT_KEY_F7:  Key_Event(K_F7,  s); break;
        case GLUT_KEY_F8:  Key_Event(K_F8,  s); break;
        case GLUT_KEY_F9:  Key_Event(K_F9,  s); break;
        case GLUT_KEY_F10: Key_Event(K_F10, s); break;
        case GLUT_KEY_F11: Key_Event(K_F11, s); break;
        case GLUT_KEY_F12: Key_Event(K_F12, s); break;

        case GLUT_KEY_LEFT:  Key_Event(K_LEFTARROW,  s); break;
        case GLUT_KEY_RIGHT: Key_Event(K_RIGHTARROW, s); break;
        case GLUT_KEY_UP:    Key_Event(K_UPARROW,    s); break;
        case GLUT_KEY_DOWN:  Key_Event(K_DOWNARROW,  s); break;

        case GLUT_KEY_PAGE_UP:   Key_Event(K_PGUP, s); break;
        case GLUT_KEY_PAGE_DOWN: Key_Event(K_PGDN, s); break;
        case GLUT_KEY_HOME:      Key_Event(K_HOME, s); break;
        case GLUT_KEY_END:       Key_Event(K_END,  s); break;
        case GLUT_KEY_INSERT:    Key_Event(K_INS,  s); break;
    }
}
void skey_down_handler(int key, int x, int y) {
    skey_handler(key, 1);
}
void skey_up_handler(int key, int x, int y) {
    skey_handler(key, 0);
}

void VID_Shutdown(void)
{
	if (!glut_window)
		return;

	glutDestroyWindow(glut_window);
}

void signal_handler(int sig)
{
	printf("Received signal %d, exiting...\n", sig);
	Sys_Quit();
	exit(0);
}

void InitSig(void)
{
	signal(SIGHUP, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGILL, signal_handler);
	signal(SIGTRAP, signal_handler);
	signal(SIGIOT, signal_handler);
	signal(SIGBUS, signal_handler);
	signal(SIGFPE, signal_handler);
	signal(SIGSEGV, signal_handler);
	signal(SIGTERM, signal_handler);
}

void VID_ShiftPalette(unsigned char *p)
{
//	VID_SetPalette(p);
}

void	VID_SetPalette (unsigned char *palette)
{
	byte	*pal;
	unsigned r,g,b;
	unsigned v;
	int     r1,g1,b1;
	int		k;
	unsigned short i;
	unsigned	*table;
	int dist, bestdist;

//
// 8 8 8 encoding
//
	pal = palette;
	table = d_8to24table;
	for (i=0 ; i<256 ; i++)
	{
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;
		
		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		*table++ = v;
	}
	d_8to24table[255] &= 0xffffff;	// 255 is transparent

	// JACK: 3D distance calcs - k is last closest, l is the distance.
	for (i=0; i < (1<<15); i++) {
		/* Maps
		000000000000000
		000000000011111 = Red  = 0x1F
		000001111100000 = Blue = 0x03E0
		111110000000000 = Grn  = 0x7C00
		*/
		r = ((i & 0x1F) << 3)+4;
		g = ((i & 0x03E0) >> 2)+4;
		b = ((i & 0x7C00) >> 7)+4;
		pal = (unsigned char *)d_8to24table;
		for (v=0,k=0,bestdist=10000*10000; v<256; v++,pal+=4) {
			r1 = (int)r - (int)pal[0];
			g1 = (int)g - (int)pal[1];
			b1 = (int)b - (int)pal[2];
			dist = (r1*r1)+(g1*g1)+(b1*b1);
			if (dist < bestdist) {
				k=v;
				bestdist = dist;
			}
		}
		d_15to8table[i]=k;
	}
}

void CheckMultiTextureExtensions(void) 
{
	void *prjobj;

	if (strstr((char *)gl_extensions, "GL_SGIS_multitexture ") && !argv->contains("-nomtex")) {
		Con_Printf("Found GL_SGIS_multitexture...\n");

		if ((prjobj = dlopen(NULL, RTLD_LAZY)) == NULL) {
			Con_Printf("Unable to open symbol list for main program.\n");
			return;
		}

		qglMTexCoord2fSGIS = (decltype(qglMTexCoord2fSGIS)) dlsym(prjobj, "glMTexCoord2fSGIS");
		qglSelectTextureSGIS = (decltype(qglSelectTextureSGIS)) dlsym(prjobj, "glSelectTextureSGIS");

		if (qglMTexCoord2fSGIS && qglSelectTextureSGIS) {
			Con_Printf("Multitexture extensions found.\n");
			gl_mtexable = true;
		} else
			Con_Printf("Symbol not found, disabled.\n");

		dlclose(prjobj);
	}
}

/*
===============
GL_Init
===============
*/
void GL_Init (void)
{
	gl_vendor = glGetString (GL_VENDOR);
	Con_Printf ("GL_VENDOR: %s\n", gl_vendor);
	gl_renderer = glGetString (GL_RENDERER);
	Con_Printf ("GL_RENDERER: %s\n", gl_renderer);

	gl_version = glGetString (GL_VERSION);
	Con_Printf ("GL_VERSION: %s\n", gl_version);
	gl_extensions = glGetString (GL_EXTENSIONS);
	Con_Printf ("GL_EXTENSIONS: %s\n", gl_extensions);

//	Con_Printf ("%s %s\n", gl_renderer, gl_version);

	CheckMultiTextureExtensions ();

	glClearColor (1,0,0,0);
	glCullFace(GL_FRONT);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.666);

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel (GL_FLAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

/*
=================
GL_BeginRendering

=================
*/
void GL_BeginRendering (int *x, int *y, int *width, int *height)
{
	extern quake::cvar::normal gl_clear;

	*x = *y = 0;
	*width = scr_width;
	*height = scr_height;

//    if (!wglMakeCurrent( maindc, baseRC ))
//		Sys_Error ("wglMakeCurrent failed");

//	glViewport (*x, *y, *width, *height);
}


void GL_EndRendering (void)
{
	glFlush();
    glutSwapBuffers();
}

void Init_KBD(void)
{
	if (argv->contains("-nokbd")) UseKeyboard = 0;

	if (UseKeyboard)
	{
        glutIgnoreKeyRepeat(1);
        glutKeyboardFunc(key_down_handler);
        glutKeyboardUpFunc(key_up_handler);
        glutSpecialFunc(skey_down_handler);
        glutSpecialUpFunc(skey_up_handler);
	}
}

qboolean VID_Is8bit(void)
{
	return is8bit;
}

void VID_Init8bitPalette(void) 
{
	// Check for 8bit Extensions and initialize them.
	int i;
	void *prjobj;

	if (argv->contains("-no8bit"))
		return;

	if ((prjobj = dlopen(NULL, RTLD_LAZY)) == NULL) {
		Con_Printf("Unable to open symbol list for main program.\n");
		return;
	}

	if (strstr((char *)gl_extensions, "3DFX_set_global_palette") &&
		(qgl3DfxSetPaletteEXT = (decltype(qgl3DfxSetPaletteEXT))dlsym(prjobj, "gl3DfxSetPaletteEXT")) != NULL) {
		GLubyte table[256][4];
		char *oldpal;

		Con_SafePrintf("... Using 3DFX_set_global_palette\n");
		glEnable( GL_SHARED_TEXTURE_PALETTE_EXT );
		oldpal = (char *) d_8to24table; //d_8to24table3dfx;
		for (i=0;i<256;i++) {
			table[i][2] = *oldpal++;
			table[i][1] = *oldpal++;
			table[i][0] = *oldpal++;
			table[i][3] = 255;
			oldpal++;
		}
		qgl3DfxSetPaletteEXT((GLuint *)table);
		is8bit = true;

	} else if (strstr((char *)gl_extensions, "GL_EXT_shared_texture_palette") &&
		(qglColorTableEXT = (decltype(qglColorTableEXT))dlsym(prjobj, "glColorTableEXT")) != NULL) {
		char thePalette[256*3];
		char *oldPalette, *newPalette;

		Con_SafePrintf("... Using GL_EXT_shared_texture_palette\n");
		glEnable( GL_SHARED_TEXTURE_PALETTE_EXT );
		oldPalette = (char *) d_8to24table; //d_8to24table3dfx;
		newPalette = thePalette;
		for (i=0;i<256;i++) {
			*newPalette++ = *oldPalette++;
			*newPalette++ = *oldPalette++;
			*newPalette++ = *oldPalette++;
			oldPalette++;
		}
		qglColorTableEXT(GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB, 256, GL_RGB, GL_UNSIGNED_BYTE, (void *) thePalette);
		is8bit = true;
	
	}

	dlclose(prjobj);
}

static void Check_Gamma (unsigned char *pal)
{
	float	f, inf;
	unsigned char	palette[768];
	int		i;

    vid_gamma = std::stof(argv->get_or_default("-gamma", "0.7"));

	for (i=0 ; i<768 ; i++)
	{
		f = pow ( (pal[i]+1)/256.0 , vid_gamma );
		inf = f*255 + 0.5;
		if (inf < 0)
			inf = 0;
		if (inf > 255)
			inf = 255;
		palette[i] = inf;
	}

	memcpy (pal, palette, sizeof(palette));
}

void VID_Init(unsigned char *palette)
{
	char	gldir[MAX_OSPATH];

	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

// interpret command-line params

    int width  = std::stoi(argv->get_or_default("-width",  "640"));
    int height = std::stoi(argv->get_or_default("-height", "480"));

    vid.conwidth = std::stoi(argv->get_or_default("-conwidth", "640"));
	vid.conwidth &= 0xfff8; // make it a multiple of eight

	if (vid.conwidth < 320)
		vid.conwidth = 320;

	// pick a conheight that matches with correct aspect
    vid.conheight = std::stoi(argv->get_or_default("-conheight", std::to_string(vid.conwidth * 3 / 4)));
	if (vid.conheight < 200)
		vid.conheight = 200;

    int dummy_argc = 0;
    glutInit(&dummy_argc, nullptr);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(width, height);
    glut_window = glutCreateWindow("Quake");

	Init_KBD();

	InitSig(); // trap evil signals

	scr_width = width;
	scr_height = height;

	if (vid.conheight > height)
		vid.conheight = height;
	if (vid.conwidth > width)
		vid.conwidth = width;
	vid.width = vid.conwidth;
	vid.height = vid.conheight;

	vid.aspect = ((float)vid.height / (float)vid.width) *
				(320.0 / 240.0);
	vid.numpages = 2;

	GL_Init();

	sprintf (gldir, "%s/glquake", com_gamedir);
	Sys_mkdir (gldir);

	Check_Gamma(palette);
	VID_SetPalette(palette);

	// Check for 3DFX Extensions and initialize them.
	VID_Init8bitPalette();

	Con_SafePrintf ("Video mode %dx%d initialized.\n", width, height);

	vid.recalc_refdef = 1;				// force a surface cache flush
}

void Sys_SendKeyEvents(void)
{
}

void Force_CenterView_f (const quake::common::argv & args)
{
	cl.viewangles[PITCH] = 0;
}

void mousehandler(int button, int state, int x, int y)
{
	mouse_buttonstate[button] = state;
}

int ox = 0xcafebabe, oy;
void mouse_motion(int x, int y) {
    if (ox != 0xcafebabe) {
	    mx += (x - ox);
	    my += (y - oy);
    }
    ox = x;
    oy = y;
}

void IN_Init(void)
{
	if (UseMouse)
	{

		Cmd_AddCommand ("force_centerview", Force_CenterView_f);

		mouse_buttons = 3;

        glutMotionFunc(mouse_motion);
        glutMouseFunc(mousehandler);
	}

}

void IN_Shutdown(void)
{
}

/*
===========
IN_Commands
===========
*/
void IN_Commands (void)
{
	if (UseMouse && cls.state != ca_dedicated)
	{
		// perform button actions
		if (mouse_buttonstate[0] && !mouse_oldbuttonstate[0])
			Key_Event (K_MOUSE1, true);
		else if (!mouse_buttonstate[0] && mouse_oldbuttonstate[0])
			Key_Event (K_MOUSE1, false);

		if (mouse_buttonstate[1] && !mouse_oldbuttonstate[1])
			Key_Event (K_MOUSE2, true);
		else if (!mouse_buttonstate[1] && mouse_oldbuttonstate[1])
			Key_Event (K_MOUSE2, false);

		if (mouse_buttonstate[2] && !mouse_oldbuttonstate[2])
			Key_Event (K_MOUSE3, true);
		else if (!mouse_buttonstate[2] && mouse_oldbuttonstate[2])
			Key_Event (K_MOUSE3, false);

		mouse_oldbuttonstate[0] = mouse_buttonstate[0];
		mouse_oldbuttonstate[1] = mouse_buttonstate[1];
		mouse_oldbuttonstate[2] = mouse_buttonstate[2];
	}
}

/*
===========
IN_Move
===========
*/
void IN_MouseMove (usercmd_t *cmd)
{
	if (!UseMouse)
		return;

	if (m_filter.to_bool())
	{
		mouse_x = (mx + old_mouse_x) * 0.5;
		mouse_y = (my + old_mouse_y) * 0.5;
	}
	else
	{
		mouse_x = mx;
		mouse_y = my;
	}
	old_mouse_x = mx;
	old_mouse_y = my;
	mx = my = 0; // clear for next update

	mouse_x *= sensitivity.to_float();
	mouse_y *= sensitivity.to_float();

// add mouse X/Y movement to cmd
	if ( (in_strafe.state & 1) || (lookstrafe.to_bool() && (in_mlook.state & 1) ))
		cmd->sidemove += m_side.to_float() * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw.to_float() * mouse_x;
	
	if (in_mlook.state & 1)
		V_StopPitchDrift ();
		
	if ( (in_mlook.state & 1) && !(in_strafe.state & 1))
	{
		cl.viewangles[PITCH] += m_pitch.to_float() * mouse_y;
		if (cl.viewangles[PITCH] > 80)
			cl.viewangles[PITCH] = 80;
		if (cl.viewangles[PITCH] < -70)
			cl.viewangles[PITCH] = -70;
	}
	else
	{
		if ((in_strafe.state & 1) && noclip_anglehack)
			cmd->upmove -= m_forward.to_float() * mouse_y;
		else
			cmd->forwardmove -= m_forward.to_float() * mouse_y;
	}
}

void IN_Move (usercmd_t *cmd)
{
	IN_MouseMove(cmd);
}


