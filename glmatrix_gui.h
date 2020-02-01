#ifndef _SETTINGS_WINDOW__
#define _SETTINGS_WINDOW__

#define Timeout			1
#define Timeout_Integer	2
#define Density			3
#define Density_Integer	4
#define Speed			5
#define Speed_Integer	6
#define Encoding		7
#define ScreenMode		8
#define Fog				9
#define Wave			10
#define Rotate			11
#define Invert			12
#define Save			13
#define Use				14
#define Test			15
#define Cancel			16

extern BOOL gui_init(void);
extern void gui_destroy(void);
extern void gui_open(void);
extern void gui_close(void);
extern void gui_handle_window(void);

extern ULONG gui_window_signals;

#endif

