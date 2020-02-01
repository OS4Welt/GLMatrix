#ifndef __GL_MATRIX__
#define __GL_MATRIX__

#include <mgl/gl.h>

#define GRID_SIZE  70     /* width and height of the arena */
#define GRID_DEPTH 35     /* depth of the arena */
#define WAVE_SIZE  22     /* periodicity of color (brightness) waves */
#define SPLASH_RATIO 0.7  /* ratio of GRID_DEPTH where chars hit the screen */

#ifdef __GNUC__
   #ifdef __PPC__
    #pragma pack(2)
   #endif
#elif defined(__VBCC__)
   #pragma amiga-align
#endif

typedef struct {
  GLfloat x, y, z;        /* position of strip */
  GLfloat dx, dy, dz;     /* velocity of strip */

  int spinner_glyph;      /* the bottommost glyph -- the feeder */
  GLfloat spinner_y;      /* where on the strip the bottom glyph is */
  GLfloat spinner_speed;  /* how fast the bottom glyph drops */

  int glyphs[GRID_SIZE];  /* the other glyphs on the strip, which will be
                             revealed by the dropping spinner.
                             0 means no glyph; negative means "spinner".
                             If non-zero, real value is abs(G)-1. */

  int spin_speed;         /* Rotate all spinners every this-many frames */
  int spin_tick;          /* frame counter */

  int wave_position;	  /* Waves of brightness wash down the strip. */
  int wave_speed;	  /* every this-many frames. */
  int wave_tick;	  /* frame counter. */

  unsigned int erasing_p;         /* Whether this strip is on its way out. */

} strip;

typedef struct {
  GLuint texture;
  int nstrips;
  strip *strips;
  int *glyph_map;
  int nglyphs;
  GLfloat tex_char_width, tex_char_height;

  /* auto-tracking direction of view */
  int last_view, target_view;
  GLfloat view_x, view_y;
  int view_steps, view_tick;

  unsigned int button_down_p;
  unsigned int auto_tracking_p;

} matrix_configuration;

extern matrix_configuration *mps;
extern void init_matrix(void);
extern void draw_matrix(matrix_configuration *mps);

#ifdef __GNUC__
   #ifdef __PPC__
    #pragma pack()
   #endif
#elif defined(__VBCC__)
   #pragma default-align
#endif

#endif

