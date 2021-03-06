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
// disable data conversion warnings

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "quake/cvar.hpp"

namespace quake {
    class texture;
}

void GL_BeginRendering (int *x, int *y, int *width, int *height);
void GL_EndRendering (void);


extern	int		texture_mode;

extern	float	gldepthmin, gldepthmax;

void GL_Upload32 (unsigned *data, int width, int height,  qboolean mipmap, qboolean alpha);
void GL_Upload8 (byte *data, int width, int height,  qboolean mipmap, qboolean alpha);
std::shared_ptr<quake::texture> GL_LoadTexture (const char *identifier, int width, int height, byte *data, qboolean mipmap, qboolean alpha);

typedef struct
{
	float	x, y, z;
	float	s, t;
	float	r, g, b;
} glvert_t;

extern glvert_t glv;

extern	int glx, gly, glwidth, glheight;

// r_local.h -- private refresh defs

#define ALIAS_BASE_SIZE_RATIO		(1.0 / 11.0)
					// normalizing factor so player model works out to about
					//  1 pixel per triangle
#define	MAX_LBM_HEIGHT		480

#define TILE_SIZE		128		// size of textures generated by R_GenTiledSurf

#define SKYSHIFT		7
#define	SKYSIZE			(1 << SKYSHIFT)
#define SKYMASK			(SKYSIZE - 1)

#define BACKFACE_EPSILON	0.01


void R_TimeRefresh_f (const quake::common::argv & argv);
void R_ReadPointFile_f (const quake::common::argv & argv);
texture_t *R_TextureAnimation (texture_t *base);

typedef struct surfcache_s
{
	struct surfcache_s	*next;
	struct surfcache_s 	**owner;		// NULL is an empty chunk of memory
	int					lightadj[MAXLIGHTMAPS]; // checked for strobe flush
	int					dlight;
	int					size;		// including header
	unsigned			width;
	unsigned			height;		// DEBUG only needed for debug
	float				mipscale;
	struct texture_s	*texture;	// checked for animating textures
	byte				data[4];	// width*height elements
} surfcache_t;


typedef struct
{
	pixel_t		*surfdat;	// destination for generated surface
	int			rowbytes;	// destination logical width in bytes
	msurface_t	*surf;		// description for surface to generate
	fixed8_t	lightadj[MAXLIGHTMAPS];
							// adjust for lightmap levels for dynamic lighting
	texture_t	*texture;	// corrected for animating textures
	int			surfmip;	// mipmapped ratio of surface texels / world pixels
	int			surfwidth;	// in mipmapped texels
	int			surfheight;	// in mipmapped texels
} drawsurf_t;


typedef enum {
	pt_static, pt_grav, pt_slowgrav, pt_fire, pt_explode, pt_explode2, pt_blob, pt_blob2
} ptype_t;

// !!! if this is changed, it must be changed in d_ifacea.h too !!!
typedef struct particle_s
{
// driver-usable fields
	vec3_t		org;
	float		color;
// drivers never touch the following fields
	struct particle_s	*next;
	vec3_t		vel;
	float		ramp;
	float		die;
	ptype_t		type;
} particle_t;


//====================================================


extern	entity_t	r_worldentity;
extern	qboolean	r_cache_thrash;		// compatability
extern	vec3_t		modelorg, r_entorigin;
extern	entity_t	*currententity;
extern	int			r_visframecount;	// ??? what difs?
extern	int			r_framecount;
extern	mplane_t	frustum[4];
extern	int		c_brush_polys, c_alias_polys;


//
// view origin
//
extern	vec3_t	vup;
extern	vec3_t	vpn;
extern	vec3_t	vright;
extern	vec3_t	r_origin;

//
// screen size info
//
extern	refdef_t	r_refdef;
extern	mleaf_t		*r_viewleaf, *r_oldviewleaf;
extern	texture_t	*r_notexture_mip;
extern	int		d_lightstylevalue[256];	// 8.8 fraction of base light value

extern	qboolean	envmap;
extern	int	cnttextures[2];
extern	GLuint particletexture;
extern	GLuint playertextures[16];

extern	int	skytexturenum;		// index in cl.loadmodel, not gl texture object

extern quake::cvar::normal r_norefresh;
extern quake::cvar::normal r_drawentities;
extern quake::cvar::normal r_drawworld;
extern quake::cvar::normal r_drawviewmodel;
extern quake::cvar::normal r_speeds;
extern quake::cvar::normal r_waterwarp;
extern quake::cvar::normal r_fullbright;
extern quake::cvar::normal r_lightmap;
extern quake::cvar::normal r_shadows;
extern quake::cvar::normal r_mirroralpha;
extern quake::cvar::normal r_wateralpha;
extern quake::cvar::normal r_dynamic;
extern quake::cvar::normal r_novis;

extern quake::cvar::normal gl_clear;
extern quake::cvar::normal gl_cull;
extern quake::cvar::normal gl_poly;
extern quake::cvar::normal gl_smoothmodels;
extern quake::cvar::normal gl_affinemodels;
extern quake::cvar::normal gl_polyblend;
extern quake::cvar::normal gl_keeptjunctions;
extern quake::cvar::normal gl_reporttjunctions;
extern quake::cvar::normal gl_flashblend;
extern quake::cvar::normal gl_nocolors;
extern quake::cvar::normal gl_doubleeyes;

extern	int		gl_lightmap_format;
extern	int		gl_solid_format;
extern	int		gl_alpha_format;

extern	int			mirrortexturenum;	// quake texturenum, not gltexturenum
extern	qboolean	mirror;
extern	mplane_t	*mirror_plane;

extern	float	r_world_matrix[16];

extern	const GLubyte *gl_vendor;
extern	const GLubyte *gl_renderer;
extern	const GLubyte *gl_version;
extern	const GLubyte *gl_extensions;

void R_TranslatePlayerSkin (int playernum);

#ifndef _WIN32
#define APIENTRY /* */
#endif

