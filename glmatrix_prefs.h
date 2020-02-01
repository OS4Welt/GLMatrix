#ifndef __GLMATRIX_PREFS__
#define __GLMATRIX_PREFS__

#include <exec/types.h>
#include <libraries/iffparse.h>

#define ID_GLMP MAKE_ID('G','L','M','P')
#define ENVPREFSFILE "env:glmatrix.prefs"
#define ENVARCPREFSFILE "envarc:glmatrix.prefs"

#ifdef __GNUC__
   #ifdef __PPC__
    #pragma pack(2)
   #endif
#elif defined(__VBCC__)
   #pragma amiga-align
#endif

struct GLMatrixPrefs
{
	ULONG glm_Timeout;
	ULONG glm_Density;
	ULONG glm_Speed;
	ULONG glm_Encoding;
	BOOL  glm_Fog;
	BOOL  glm_Wave;
	BOOL  glm_Rotate;
	BOOL  glm_Invert;
	ULONG glm_ScreenModeID;
};

extern struct GLMatrixPrefs *glmatrix_prefs;

extern struct GLMatrixPrefs* read_prefs(void);
extern void use_prefs(void);
extern void save_prefs(void);

#ifdef __GNUC__
   #ifdef __PPC__
    #pragma pack()
   #endif
#elif defined(__VBCC__)
   #pragma default-align
#endif

#endif

