/* glmatrix, Copyright (c) 2003 Jamie Zawinski <jwz@jwz.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *
 * GLMatrix -- simulate the text scrolls from the movie "The Matrix".
 *
 * This program does a 3D rendering of the dropping characters that
 * appeared in the title sequences of the movies.  See also `xmatrix'
 * for a simulation of what the computer monitors actually *in* the
 * movie did.
 */


#define progname "glmatrix"

/*#define DEF_SPEED       "1.0"
#define DEF_DENSITY     "20"
#define DEF_FOG         "True"
#define DEF_WAVES       "True"
#define DEF_ROTATE      "True"
#define DEF_TEXTURE     "True"
#define DEF_MODE        "Matrix"

#define DEFAULTS	"*delay:	30000         \n" \
			"*showFPS:      FALSE         \n" \
			"*wireframe:    FALSE         \n" \
			"*mode:       " DEF_MODE    " \n" \
			"*speed:      " DEF_SPEED   " \n" \
			"*density:    " DEF_DENSITY " \n" \
			"*fog:        " DEF_FOG     " \n" \
			"*waves:      " DEF_WAVES   " \n" \
			"*texture:    " DEF_TEXTURE " \n" \
			"*rotate:     " DEF_ROTATE  " \n" \
*/

#undef countof
#define countof(x) (sizeof((x))/sizeof((*x)))

#undef BELLRAND
#define BELLRAND(n) ((frand((n)) + frand((n)) + frand((n))) / 3.0)

/*#include "xlockmore.h"*/
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>

#include "glmatrix.h"
#include "yarandom.h"

#include "glmatrix_prefs.h"

#ifdef __GNUC__
  __extension__  /* don't warn about "string length is greater than the length
                    ISO C89 compilers are required to support" when including
                    the following XPM file... */
#endif

#ifndef M_PI
#define M_PI 3.1415627165242
#endif

#define USE_GL 1

#ifdef USE_GL /* whole file */

/*#include <mgl/gl.h>*/


/*#include "gllist.h"*/


#define CHAR_COLS 16
#define CHAR_ROWS 13
static int real_char_rows;

static int matrix_encoding[] = { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                                 192, 193, 194, 195, 196, 197, 198, 199,
                                 200, 201, 202, 203, 204, 205, 206, 207 };
static int decimal_encoding[]  = { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 };
static int hex_encoding[]      = { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                                   33, 34, 35, 36, 37, 38 };
static int binary_encoding[] = { 16, 17 };
static int dna_encoding[]    = { 33, 35, 39, 52 };
#if 0
static unsigned char char_map[256] = {
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  /*   0 */
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  /*  16 */
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,  /*  32 */
   16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,  /*  48 */
   32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,  /*  64 */
   48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,  /*  80 */
   64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,  /*  96 */
   80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,  /* 112 */
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  /* 128 */
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  /* 144 */
   96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,  /* 160 */
  112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,  /* 176 */
  128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,  /* 192 */
  144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,  /* 208 */
  160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,  /* 224 */
  176,177,178,195,180,181,182,183,184,185,186,187,188,189,190,191   /* 240 */
};
#endif /* 0 */

#define CURSOR_GLYPH 97

/*#define DEBUG*/

static struct { GLfloat x, y; } nice_views[] = {
  {  0,     0 },
  {  0,   -20 },     /* this is a list of viewer rotations that look nice. */
  {  0,    20 },     /* every now and then we switch to a new one.         */
  { 25,     0 },     /* (but we only use the first one at start-up.)       */
  {-25,     0 },
  { 25,    20 },
  {-25,    20 },
  { 25,   -20 },
  {-25,   -20 },

  { 10,     0 },
  {-10,     0 },
  {  0,     0 },  /* prefer these */
  {  0,     0 },
  {  0,     0 },
  {  0,     0 },
  {  0,     0 },
};

typedef struct {
	int width;
	int height;
	int bytes_per_line;
	unsigned long *data;
} XImage;


matrix_configuration *mps = NULL;

static GLfloat brightness_ramp[WAVE_SIZE];

static GLfloat speed = 1.0f;
static GLfloat density = 20.0f;
static BOOL do_fog = TRUE;
static BOOL do_waves = TRUE;
static BOOL do_rotate = TRUE;
static BOOL do_texture = TRUE;
static char *mode_str = "matrix";

#ifndef STAND_ALONE
extern int width;
extern int height;
#else
int width = 640;
int height = 480;
#endif

/*
static XrmOptionDescRec opts[] = {
  { "-speed",       ".speed",     XrmoptionSepArg, 0 },
  { "-density",     ".density",   XrmoptionSepArg, 0 },
  { "-mode",        ".mode",      XrmoptionSepArg, 0 },
  { "-binary",      ".mode",      XrmoptionNoArg, "binary"      },
  { "-hexadecimal", ".mode",      XrmoptionNoArg, "hexadecimal" },
  { "-decimal",     ".mode",      XrmoptionNoArg, "decimal"     },
  { "-dna",         ".mode",      XrmoptionNoArg, "dna"         },
  { "-fog",         ".fog",       XrmoptionNoArg, "True"  },
  { "+fog",         ".fog",       XrmoptionNoArg, "FALSE" },
  { "-waves",       ".waves",     XrmoptionNoArg, "True"  },
  { "+waves",       ".waves",     XrmoptionNoArg, "FALSE" },
  { "-rotate",      ".rotate",    XrmoptionNoArg, "True"  },
  { "+rotate",      ".rotate",    XrmoptionNoArg, "FALSE" },
  {"-texture",      ".texture",   XrmoptionNoArg, "True"  },
  {"+texture",      ".texture",   XrmoptionNoArg, "FALSE" },
};
*/

/*
static argtype vars[] = {
  {(caddr_t *) &mode_str,   "mode",       "Mode",    DEF_MODE,      t_String},
  {(caddr_t *) &speed,      "speed",      "Speed",   DEF_SPEED,     t_Float},
  {(caddr_t *) &density,    "density",    "Density", DEF_DENSITY,   t_Float},
  {(caddr_t *) &do_fog,     "fog",        "Fog",     DEF_FOG,       t_Bool},
  {(caddr_t *) &do_waves,   "waves",      "Waves",   DEF_WAVES,     t_Bool},
  {(caddr_t *) &do_rotate,  "rotate",     "Rotate",  DEF_ROTATE,    t_Bool},
  {(caddr_t *) &do_texture, "texture",    "Texture", DEF_TEXTURE,   t_Bool},
};


ModeSpecOpt matrix_opts = {countof(opts), opts, countof(vars), vars, NULL};
*/

/* Re-randomize the state of one strip.
 */
static void reset_strip(matrix_configuration *mp, strip *s)
{
	/*matrix_configuration *mp = &mps[MI_SCREEN(mi)];*/
	int i;

	memset (s, 0, sizeof(*s));
	s->x = (GLfloat) (frand((double)GRID_SIZE) - (GRID_SIZE / 2.0));
	s->y = (GLfloat) (GRID_SIZE/2.0 + BELLRAND(0.5));      /* shift top slightly */
	s->z = (GLfloat) (GRID_DEPTH * 0.2) - frand (GRID_DEPTH * 0.7);
	s->spinner_y = 0;

	s->dx = 0;
/*	s->dx = ((BELLRAND(0.01) - 0.005) * speed); */
	s->dy = 0;
	s->dz = (BELLRAND(0.02) * speed);

	s->spinner_speed = (BELLRAND(0.3) * speed);

	s->spin_speed = (int) BELLRAND(2.0 / speed) + 1;
	s->spin_tick  = 0;

	s->wave_position = 0;
	s->wave_speed = (int) BELLRAND(3.0 / speed) + 1;
	s->wave_tick  = 0;

	for (i = 0; i < GRID_SIZE; i++)
	{
		int draw_p = (random() % 7);
		int spin_p = (draw_p && !(random() % 20));
		int g = (draw_p
					? mp->glyph_map[(random() % mp->nglyphs)] + 1
					: 0);

		if (spin_p) g = -g;
		s->glyphs[i] = g;
	}

	s->spinner_glyph = - (mp->glyph_map[(random() % mp->nglyphs)] + 1);
}


/* Animate the strip one step.  Reset if it has reached the bottom.
 */
static void tick_strip(matrix_configuration *mp, strip *s)
{
/*  matrix_configuration *mp = &mps[MI_SCREEN(mi)];*/
	int i;

	if (mp->button_down_p)
		return;

	s->x += s->dx;
	s->y += s->dy;
	s->z += s->dz;

	if (s->z > GRID_DEPTH * SPLASH_RATIO)  /* splashed into screen */
	{
		reset_strip (mp, s);
		return;
	}

	s->spinner_y += s->spinner_speed;
	if (s->spinner_y >= GRID_SIZE)
	{
		if (s->erasing_p)
		{
			reset_strip (mp, s);
			return;
		}
		else
		{
			s->erasing_p = TRUE;
			s->spinner_y = 0;
			s->spinner_speed /= 2;  /* erase it slower than we drew it */
		}
	}

	/* Spin the spinners. */
	s->spin_tick++;
	if (s->spin_tick > s->spin_speed)
	{
		s->spin_tick = 0;
		s->spinner_glyph = - (mp->glyph_map[(random() % mp->nglyphs)] + 1);
		for (i = 0; i < GRID_SIZE; i++)
			if (s->glyphs[i] < 0)
			{
				s->glyphs[i] = -(mp->glyph_map[(random() % mp->nglyphs)] + 1);
				if (! (random() % 800))  /* sometimes they stop spinning */
					s->glyphs[i] = -s->glyphs[i];
			}
		}

	/* Move the color (brightness) wave. */
	s->wave_tick++;
	if (s->wave_tick > s->wave_speed)
	{
		s->wave_tick = 0;
		s->wave_position++;
		if (s->wave_position >= WAVE_SIZE)
			s->wave_position = 0;
	}
}


/* Draw a single character at the given position and brightness.
 */
static void draw_glyph(matrix_configuration *mp, int glyph,
								GLfloat x, GLfloat y, GLfloat z,
								GLfloat brightness)
{
	/*matrix_configuration *mp = &mps[MI_SCREEN(mi)];*/
	GLfloat w = mp->tex_char_width;
	GLfloat h = mp->tex_char_height;
	GLfloat cx = 0, cy = 0;
	GLfloat S = 1;
	BOOL spinner_p = (glyph < 0);

	if (glyph == 0) abort();
	if (glyph < 0) glyph = -glyph;

	if (spinner_p)
		brightness *= 1.5f;

	if (!do_texture)
	{
		S  = 0.8;
		x += 0.1;
		y += 0.1;
	}
	else
	{
		int ccx = ((glyph - 1) % CHAR_COLS);
		int ccy = ((glyph - 1) / CHAR_COLS);
		cx = ccx * w;
		cy = (real_char_rows - ccy - 1) * h;

		if (do_fog)
		{
			GLfloat depth;
			depth = (z / GRID_DEPTH) + 0.5f;  /* z ratio from back/front      */
			depth = 0.2f + (depth * 0.8f);     /* scale to range [0.2 - 1.0]   */
			brightness *= depth;             /* so no row goes all black.    */
		}
	}

	{
		GLfloat r, g, b, a = 1;
		if (!do_texture && !spinner_p)
		{
			r = b = 0, g = brightness;
		}
		else
		{
			r = g = b = brightness;
		}

		/*	If the glyph is very close to the screen (meaning it is very large,
			and is about to splash into the screen and vanish) then start fading
			it out, proportional to how close to the glass it is.
		*/

		if (z > GRID_DEPTH/2)
		{
			GLfloat ratio = ((z - GRID_DEPTH/2) / ((GRID_DEPTH * SPLASH_RATIO) - GRID_DEPTH/2));
			int i = ratio * WAVE_SIZE;

			if (i < 0)
			{
				i = 0;
			}
			else if (i >= WAVE_SIZE)
			{
				i = WAVE_SIZE-1;
			}

			a = brightness_ramp[i];
#if 1
			/*	I don't understand this -- if I change the alpha on the color of
				the quad, I'd expect that to make the quad more transparent.
				But instead, it seems to be making the transparent parts of the
				texture on the quad be *less* transparent!  So as we fade out,
				we fade towards a completely solid rectangle.  WTF?

				So, for now, instead of changing the alpha, just make the colors
				be darker.  This isn't quite right (it causes a large dark glyph
				to occlude the brighter glyphs behind it) but it's close...
			*/

			r *= a;
			g *= a;
			b *= a;
			a = 1;
#endif
		}


		glColor4f(r,g,b,a);
	}

	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glTexCoord2f(cx,     cy);     glVertex3f (x,     y,     z);
	glTexCoord2f(cx + w, cy);     glVertex3f (x + S, y,     z);
	glTexCoord2f(cx + w, cy + h); glVertex3f (x + S, y + S, z);
	glTexCoord2f(cx,     cy + h); glVertex3f (x,     y + S, z);
	glEnd ();

	/*mi->polygon_count++;*/
}


/* Draw all the visible glyphs in the strip.
 */
static void draw_strip(matrix_configuration *mp, strip *s)
{
	int i;
	for (i = 0; i < GRID_SIZE; i++)
	{
		int g = s->glyphs[i];
		BOOL below_p = (s->spinner_y >= i);

		if (s->erasing_p)
			below_p = !below_p;

		if (g && below_p)       /* don't draw cells below the spinner */
		{
			GLfloat brightness = 1.0f;
			if (do_waves)
			{
				int j = WAVE_SIZE - ((i + (GRID_SIZE - s->wave_position)) % WAVE_SIZE);
				brightness = brightness_ramp[j];
			}

			draw_glyph(mp, g, s->x, s->y - i, s->z, brightness);
		}
	}

	if (!s->erasing_p)
		draw_glyph(mp, s->spinner_glyph, s->x, s->y - s->spinner_y, s->z, 1.0);
}


/* qsort comparator for sorting strips by z position */
static int
cmp_strips (const void *aa, const void *bb)
{
  const strip *a = *(strip **) aa;
  const strip *b = *(strip **) bb;
  return ((int) (a->z * 10000) -
          (int) (b->z * 10000));
}


/* Auto-tracking
 */

static void auto_track_init(matrix_configuration *mp)
{
	/*matrix_configuration *mp = &mps[MI_SCREEN(mi)];*/
	mp->last_view = 0;
	mp->target_view = 0;
	mp->view_x = nice_views[mp->last_view].x;
	mp->view_y = nice_views[mp->last_view].y;
	mp->view_steps = 100;
	mp->view_tick = 0;
	mp->auto_tracking_p = FALSE;
}

static void auto_track(matrix_configuration *mp)
{
	/*matrix_configuration *mp = &mps[MI_SCREEN(mi)];*/

	if (! do_rotate)
		return;
	if (mp->button_down_p)
		return;

	/* if we're not moving, maybe start moving.  Otherwise, do nothing. */
	if (! mp->auto_tracking_p)
	{
		static int tick = 0;
		if (++tick < 20/speed) return;
		tick = 0;
		if (! (random() % 20))
			mp->auto_tracking_p = TRUE;
		else
			return;
	}


	{
		GLfloat ox = nice_views[mp->last_view].x;
		GLfloat oy = nice_views[mp->last_view].y;
		GLfloat tx = nice_views[mp->target_view].x;
		GLfloat ty = nice_views[mp->target_view].y;

		/* move from A to B with sinusoidal deltas, so that it doesn't jerk
			to a stop.
		*/

		GLfloat th = sin ((M_PI / 2) * (double) mp->view_tick / mp->view_steps);

		mp->view_x = (ox + ((tx - ox) * th));
		mp->view_y = (oy + ((ty - oy) * th));
		mp->view_tick++;

		if (mp->view_tick >= mp->view_steps)
		{
			mp->view_tick = 0;
			mp->view_steps = (350.0 / speed);
			mp->last_view = mp->target_view;
			mp->target_view = (random() % (countof(nice_views) - 1)) + 1;
			mp->auto_tracking_p = FALSE;
		}
	}
}


/* Window management, etc
 */
void reshape_matrix(int width, int height)
{
	GLfloat h = (GLfloat) height / (GLfloat) width;

	glViewport(0, 0, (GLint) width, (GLint) height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80.0f, 1/h, 1.0f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(	0.0f, 0.0f, 25.0f,
					0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f);

	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);
}

/*
Bool
matrix_handle_event (ModeInfo *mi, XEvent *event)
{
  matrix_configuration *mp = &mps[MI_SCREEN(mi)];

  if (event->xany.type == ButtonPress &&
      event->xbutton.button & Button1)
    {
      mp->button_down_p = TRUE;
      return TRUE;
    }
  else if (event->xany.type == ButtonRelease &&
           event->xbutton.button & Button1)
    {
      mp->button_down_p = FALSE;
      return TRUE;
    }

  return FALSE;
}
*/

#if 1
static BOOL
bigendian (void)
{
  union { int i; char c[sizeof(int)]; } u;
  u.i = 1;
  return !u.c[0];
}
#endif


/* The image with the characters in it is 512x598, meaning that it needs to
   be copied into a 512x1024 texture.  But some machines can't handle textures
   that large...  And it turns out that we aren't using most of the characters
   in that image anyway, since this program doesn't do anything that makes use
   of the full range of Latin1 characters.  So... this function tosses out the
   last 32 of the Latin1 characters, resulting in a 512x506 image, which we
   can then stuff in a 512x512 texture.  Voila.

   If this hack ever grows into something that displays full Latin1 text,
   well then, Something Else Will Need To Be Done.
 */
static void spank_image(XImage *xi)
{
	int ch = xi->height / CHAR_ROWS;
	int cut = 2;
	unsigned char *bits = (unsigned char *) xi->data;
	unsigned char *from, *to, *s, *end;
	int L = xi->bytes_per_line * ch;
	int i;

	/* Copy row 12 into 10 (which really means, copy 2 into 0,
		since texture data is upside down.).
	*/
	to   = bits + (L * cut);
	from = bits;
	end  = from + L;
	s    = from;
	while (s < end)
		*to++ = *s++;

	/* Then, pull all the bits down by 2 rows.
	*/
	to   = bits;
	from = bits + (L * cut);
	end  = bits + (L * CHAR_ROWS);
	s    = from;
	while (s < end)
		*to++ = *s++;

	/* And clear out the rest, for good measure.
	*/
	from = bits + (L * (CHAR_ROWS - cut));
	end  = bits + (L * CHAR_ROWS);
	s    = from;
	while (s < end)
		*s++ = 0;

	xi->height -= (cut * ch);
	real_char_rows -= cut;

	/* Finally, pull the map indexes back to match the new bits.
	*/
	for (i = 0; i < countof(matrix_encoding); i++)
		if (matrix_encoding[i] > (CHAR_COLS * (CHAR_ROWS - cut)))
			matrix_encoding[i] -= (cut * CHAR_COLS);
}


unsigned long XGetPixel(XImage *xi, unsigned int x, unsigned int y)
{
	return xi->data[y*xi->width + x];
}

void XPutPixel(XImage *xi, unsigned int x, unsigned int y, unsigned long pixel)
{
	xi->data[y*xi->width + x] = pixel;
}

void XDestroyImage(XImage *xi)
{
	if (xi->data) free(xi->data);
	free(xi);
}

XImage* png_to_ximage(char *png_file)
{
	png_structp png;
	png_infop startinfo;
	png_infop endinfo;
	png_byte header[8];
	png_bytep row_pointer;
	int interlace_type;
	int compression_type;
	int filter_type;
	int bit_depth;
	int color_type;
	int y;

	FILE *fp = fopen(png_file, "rb");
	XImage *xi = NULL;

	if(fp != NULL)
	{
		xi = malloc(sizeof(XImage));
		png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		startinfo = png_create_info_struct(png);
		endinfo = png_create_info_struct(png);
		png_init_io(png, fp);

		fread(header, 8, 1, fp);
		png_sig_cmp(header, 0, 8);
		png_set_sig_bytes(png, 8);
		
		png_read_info(png, startinfo);

		png_get_IHDR(png, startinfo, (png_uint_32*)&xi->width, (png_uint_32*)&xi->height, &bit_depth, &color_type,
			     &interlace_type, &compression_type, &filter_type);

		assert(interlace_type == PNG_INTERLACE_NONE);

		if (bit_depth < 8)
		{
		   	png_set_gray_1_2_4_to_8(png);
		   	bit_depth = 8;
		}
		else if (bit_depth > 8)
		{
			png_set_strip_16(png);
		}

		if (color_type == PNG_COLOR_TYPE_PALETTE)
		{
		   	png_set_palette_to_rgb(png);
		   	color_type = PNG_COLOR_TYPE_RGB;
		}

		if (color_type == PNG_COLOR_TYPE_RGB)
		{
			png_set_filler(png, 0x00, PNG_FILLER_AFTER);
		}

		png_read_update_info(png, startinfo);
		
		xi->bytes_per_line = xi->width * 4;
		xi->data = malloc(xi->bytes_per_line * xi->height);
		row_pointer = (png_bytep)(xi->data + xi->width * (xi->height - 1));

		for (y = 0; y < xi->height; y++)
		{
		   	png_read_row(png, row_pointer, NULL);
		   	row_pointer -= xi->bytes_per_line;
		}

		png_destroy_info_struct(png, &startinfo);
		png_destroy_info_struct(png, &endinfo);
		png_destroy_read_struct(&png, NULL, NULL);

		fclose(fp);
	}

	return xi;
}

static void load_textures(matrix_configuration *mp, BOOL flip_p)
{
	/*matrix_configuration *mp = &mps[MI_SCREEN(mi)];*/
	XImage *xi;
	unsigned int x, y;
	int cw, ch;
	int orig_w, orig_h;

	/* The Matrix XPM is 512x598 -- but GL texture sizes must be powers of 2.
		So we waste some padding rows to round up.

		Steen Lund Nielsen
		Texture size on voodoo is limited to 256x256 so I have made sure the
		constructed texture fits into this limit
	*/

	xi = png_to_ximage("PROGDIR:matrix3_half2.png");
	if (NULL != xi)
	{
		orig_w = xi->width;
		orig_h = xi->height;
		real_char_rows = CHAR_ROWS;

		spank_image(xi);

		if (xi->height != 256 && xi->height != 1024)
		{
			xi->height = (xi->height < 256 ? 256 : 512);
			xi->data = realloc (xi->data, xi->height * xi->bytes_per_line);
			if (!xi->data)
			{
				fprintf(stderr, "%s: out of memory\n", progname);
				exit(1);
			}
		}

		if (xi->width != 256) abort();
		if (xi->height != 256 /*&& xi->height != 512*/) abort();

		/* char size in pixels */
		cw = orig_w / CHAR_COLS;
		ch = orig_h / CHAR_ROWS;

		/* char size in ratio of final (padded) texture size */
		mp->tex_char_width  = (GLfloat) cw / xi->width;
		mp->tex_char_height = (GLfloat) ch / xi->height;

		/* Flip each character's bits horizontally -- we could also just do this
			by reversing the texture coordinates on the quads, but on some systems
			that slows things down a lot.
		*/
		if (flip_p)
		{
			int xx, col;
			unsigned long buf[100];
			for (y = 0; y < xi->height; y++)
			{
				for (col = 0, xx = 0; col < CHAR_COLS; col++, xx += cw)
				{
					for (x = 0; x < cw; x++)
						buf[x] = XGetPixel (xi, xx+x, y);
					for (x = 0; x < cw; x++)
						XPutPixel (xi, xx+x, y, buf[cw-x-1]);
				}
			}
		}

		/*	The pixmap is a color image with no transparency.  Set the texture's
			alpha to be the green channel, and set the green channel to be 100%.
		*/
		{
			int rpos, gpos, bpos, apos;  /* bitfield positions */
#if 1
			/*	#### Cherub says that the little-endian case must be taken on MacOSX,
				or else the colors/alpha are the wrong way around.  How can
				that be the case?
			*/

		if (bigendian())
		{
			rpos = 24, gpos = 16, bpos =  8, apos =  0;
		}
		else
#endif
			rpos =  0, gpos =  8, bpos = 16, apos = 24;

			for (y = 0; y < xi->height; y++)
			{
				for (x = 0; x < xi->width; x++)
				{
					unsigned long p = XGetPixel (xi, x, y);
					unsigned char r = (p >> rpos) & 0xFF;
					unsigned char g = (p >> gpos) & 0xFF;
					unsigned char b = (p >> bpos) & 0xFF;
					unsigned char a = ~g;
#ifndef STAND_ALONE
					a = glmatrix_prefs->glm_Invert ? ~a : a;
#endif
					g = 0xFF;
					p = (r << rpos) | (g << gpos) | (b << bpos) | (a << apos);
					XPutPixel (xi, x, y, p);
				}
			}
		}

		/*	Now load the texture into GL. */
		/*	clear_gl_error(); */
		glGenTextures (1, &mp->texture);

		glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
		/*glPixelStorei (GL_UNPACK_ROW_LENGTH, xi->width);*/
		glBindTexture (GL_TEXTURE_2D, mp->texture);
		/*  check_gl_error ("texture init"); */
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, xi->width, xi->height, 0, GL_RGBA,
					GL_UNSIGNED_BYTE, xi->data);
	/*	{
			char buf[255];
			sprintf (buf, "creating %dx%d texture:", xi->width, xi->height);
			check_gl_error (buf);
		}*/

		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		/*	I'd expect CLAMP to be the thing to do here, but oddly, we get a
			faint solid green border around the texture if it is *not* REPEAT!
		*/
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	/*	check_gl_error ("texture param"); */

		/*xi->data = 0;*/ /*  don't free the texture data */
		XDestroyImage(xi);
	}
}


void init_matrix()
{
	matrix_configuration *mp;
	int wire = FALSE; /*MI_IS_WIREFRAME(mi);*/
	BOOL flip_p = FALSE;
	int i;

	if (wire)
		do_texture = FALSE;

	if (!mps)
	{
		mps = (matrix_configuration *)
		calloc (1, sizeof (matrix_configuration));
		if (!mps)
		{
			fprintf(stderr, "%s: out of memory\n", progname);
			exit(1);
		}
	}

	mp = mps;
	/*mp->glx_context = init_GL(mi);*/
	mp->button_down_p = FALSE;


#ifndef STAND_ALONE
    speed = (GLfloat)glmatrix_prefs->glm_Speed / 100.0f;
    density =(GLfloat) glmatrix_prefs->glm_Density;
    do_fog = glmatrix_prefs->glm_Fog;
    do_waves = glmatrix_prefs->glm_Wave;
    do_rotate = glmatrix_prefs->glm_Rotate;

    switch (glmatrix_prefs->glm_Encoding)
    {
        case 0:
            mode_str = "matrix";
            break;
        case 1:
            mode_str = "dna";
            break;
        case 2:
            mode_str = "bin";
            break;
        case 3:
            mode_str = "hex";
            break;
        case 4:
            mode_str = "dec";
            break;
    }
#endif

	if (!mode_str || !*mode_str || !strcasecmp(mode_str, "matrix"))
	{
		flip_p = 1;
		mp->glyph_map = matrix_encoding;
		mp->nglyphs   = countof(matrix_encoding);
	}
	else if (!strcasecmp (mode_str, "dna"))
	{
		flip_p = 0;
		mp->glyph_map = dna_encoding;
		mp->nglyphs   = countof(dna_encoding);
	}
	else if (!strcasecmp (mode_str, "bin") ||
			!strcasecmp (mode_str, "binary"))
	{
		flip_p = 0;
		mp->glyph_map = binary_encoding;
		mp->nglyphs   = countof(binary_encoding);
	}
	else if (!strcasecmp (mode_str, "hex") ||
			!strcasecmp (mode_str, "hexadecimal"))
	{
		flip_p = 0;
		mp->glyph_map = hex_encoding;
		mp->nglyphs   = countof(hex_encoding);
	}
	else if (!strcasecmp (mode_str, "dec") ||
			!strcasecmp (mode_str, "decimal"))
	{
		flip_p = 0;
		mp->glyph_map = decimal_encoding;
		mp->nglyphs   = countof(decimal_encoding);
	}
	else
	{
		fprintf (stderr,
				"%s: `mode' must be matrix, dna, binary, or hex: not `%s'\n",
				progname, mode_str);
		exit (1);
	}

	reshape_matrix(width, height);

	glShadeModel(GL_FLAT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	/*glEnable(GL_NORMALIZE);*/

	if (do_texture)
	{
		load_textures(mp, flip_p);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
	}

	/* to scale coverage-percent to strips, this number looks about right... */
	mp->nstrips = (int)(density * 2.2);
	if	(mp->nstrips < 1) mp->nstrips = 1;
	else if	(mp->nstrips > 2000) mp->nstrips = 2000;

	mp->strips = calloc(mp->nstrips, sizeof(strip));
	for (i = 0; i < mp->nstrips; i++)
	{
		strip *s = &mp->strips[i];
		reset_strip(mp, s);

		/*	If we start all strips from zero at once, then the first few seconds
			of the animation are much denser than normal.  So instead, set all
			the initial strips to erase-mode with random starting positions.
			As these die off at random speeds and are re-created, we'll get a
			more consistent density. */

		s->erasing_p = TRUE;
		s->spinner_y = frand(GRID_SIZE);
		memset (s->glyphs, 0, sizeof(s->glyphs));  /* no visible glyphs */
	}

	/*	Compute the brightness ramp.
	*/

	for (i = 0; i < WAVE_SIZE; i++)
	{
		GLfloat j = ((WAVE_SIZE - i) / (GLfloat) (WAVE_SIZE - 1));
		j *= (M_PI / 2.0f);		/* j ranges from 0.0 - PI/2  */
		j = sin(j);			/* j ranges from 0.0 - 1.0   */
		j = 0.2 + (j * 0.8);	/* j ranges from 0.2 - 1.0   */
		brightness_ramp[i] = j;	/* printf("%2d %8.2f\n", i, j); */
	}

	auto_track_init(mp);
}


#ifdef DEBUG

static void draw_grid(matrix_configuration *mp)
{
/*	if (!MI_IS_WIREFRAME(mi))
	{*/
		glDisable(GL_TEXTURE_2D);
/*		glDisable(GL_BLEND);
	} */
	glPushMatrix();
	glColor3f(1, 1, 1);
	glBegin(GL_LINES);
	glVertex3f(-GRID_SIZE, 0, 0); glVertex3f(GRID_SIZE, 0, 0);
	glVertex3f(0, -GRID_SIZE, 0); glVertex3f(0, GRID_SIZE, 0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3f(-GRID_SIZE/2, -GRID_SIZE/2, 0);
	glVertex3f(-GRID_SIZE/2,  GRID_SIZE/2, 0);
	glVertex3f( GRID_SIZE/2,  GRID_SIZE/2, 0);
	glVertex3f( GRID_SIZE/2, -GRID_SIZE/2, 0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3f(-GRID_SIZE/2, GRID_SIZE/2, -GRID_DEPTH/2);
	glVertex3f(-GRID_SIZE/2, GRID_SIZE/2,  GRID_DEPTH/2);
	glVertex3f( GRID_SIZE/2, GRID_SIZE/2,  GRID_DEPTH/2);
	glVertex3f( GRID_SIZE/2, GRID_SIZE/2, -GRID_DEPTH/2);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3f(-GRID_SIZE/2, -GRID_SIZE/2, -GRID_DEPTH/2);
	glVertex3f(-GRID_SIZE/2, -GRID_SIZE/2,  GRID_DEPTH/2);
	glVertex3f( GRID_SIZE/2, -GRID_SIZE/2,  GRID_DEPTH/2);
	glVertex3f( GRID_SIZE/2, -GRID_SIZE/2, -GRID_DEPTH/2);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(-GRID_SIZE/2, -GRID_SIZE/2, -GRID_DEPTH/2);
	glVertex3f(-GRID_SIZE/2,  GRID_SIZE/2, -GRID_DEPTH/2);
	glVertex3f(-GRID_SIZE/2, -GRID_SIZE/2,  GRID_DEPTH/2);
	glVertex3f(-GRID_SIZE/2,  GRID_SIZE/2,  GRID_DEPTH/2);
	glVertex3f( GRID_SIZE/2, -GRID_SIZE/2, -GRID_DEPTH/2);
	glVertex3f( GRID_SIZE/2,  GRID_SIZE/2, -GRID_DEPTH/2);
	glVertex3f( GRID_SIZE/2, -GRID_SIZE/2,  GRID_DEPTH/2);
	glVertex3f( GRID_SIZE/2,  GRID_SIZE/2,  GRID_DEPTH/2);
	glEnd();
	glPopMatrix();
/*	if (!MI_IS_WIREFRAME(mi))
    {*/
      glEnable(GL_TEXTURE_2D);
/*		glEnable(GL_BLEND);
    } */
}
#endif /* DEBUG */


void draw_matrix(matrix_configuration *mp)
{
/*  matrix_configuration *mp = &mps[MI_SCREEN(mi)]; */
/*  Display *dpy = MI_DISPLAY(mi); */
/*  Window window = MI_WINDOW(mi); */
	int i;

/*	if (!mp->glx_context)
	return; */

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix ();

	if (do_rotate)
	{
		glRotatef (mp->view_x, 1, 0, 0);
		glRotatef (mp->view_y, 0, 1, 0);
	}

#ifdef DEBUG
# if 0
	glScalef(0.5, 0.5, 0.5);
# endif
# if 0
	glRotatef(-30, 0, 1, 0);
# endif
	draw_grid(mp);
#endif

/*	mi->polygon_count = 0; */

	/*	Render (and tick) each strip, starting at the back
		(draw the ones farthest from the camera first, to make
		the alpha transparency work out right.)
	*/
	{
		strip **sorted = malloc (mp->nstrips * sizeof(*sorted));
		for (i = 0; i < mp->nstrips; i++)
		{
			sorted[i] = &mp->strips[i];
		}

		qsort(sorted, i, sizeof(*sorted), cmp_strips);
		for (i = 0; i < mp->nstrips; i++)
		{
			strip *s = sorted[i];
			tick_strip(mp, s);
			draw_strip(mp, s);
		}
		free(sorted);
	}

	auto_track(mp);

#if 0
	glBegin(GL_QUADS);
	glColor3f(1,1,1);
	glTexCoord2f (0,0);  glVertex3f(-15,-15,0);
	glTexCoord2f (0,1);  glVertex3f(-15,15,0);
	glTexCoord2f (1,1);  glVertex3f(15,15,0);
	glTexCoord2f (1,0);  glVertex3f(15,-15,0);
	glEnd();
#endif

	glPopMatrix ();
}

#ifdef STAND_ALONE
void key(char key)
{
	switch(key)
	{
	case 'e':
		mglExit();
		break;
	default:
		break;
	}
}

void display(void)
{
	mglLockDisplay();

	draw_matrix(mps);

	mglUnlockDisplay();
	mglSwitchDisplay();
}


int main(void)
{
	MGLInit();

	mglChooseWindowMode(FALSE);

	ya_rand_init(0);

	if (mglCreateContext(100,100, 640, 480))
	{
		mglLockMode(MGL_LOCK_MANUAL);

		init_matrix();

		mglKeyFunc(key);
		mglIdleFunc(display);
		mglMainLoop();
	}

	mglDeleteContext();
	MGLTerm();

	return 0;
}
#endif


#endif /* USE_GL */


