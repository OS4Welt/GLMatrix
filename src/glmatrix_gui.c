#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/classusr.h>
#include <intuition/icclass.h>
#include <intuition/gadgetclass.h>
#include <libraries/resource.h>
#include <utility/tagitem.h>
#include <Warp3D/Warp3D.h>

#include <classes/window.h>

#include <reaction/reaction.h>
#include <reaction/reaction_macros.h>

#include <gadgets/chooser.h>
#include <gadgets/integer.h>
#include <gadgets/layout.h>
#include <gadgets/getscreenmode.h>
#include <gadgets/slider.h>
#include <gadgets/space.h>

#include <images/label.h>

#include <proto/button.h>
#include <proto/checkbox.h>
#include <proto/chooser.h>
#include <proto/getscreenmode.h>
#include <proto/integer.h>
#include <proto/label.h>
#include <proto/layout.h>
#include <proto/slider.h>
#include <proto/space.h>
#include <proto/window.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/Warp3D.h>

#include "glmatrix_gui.h"
#include "glmatrix_prefs.h"

Object *gui_window_object = NULL;

struct MsgPort *IDCMP_port = NULL;
struct MsgPort *app_port = NULL;
ULONG gui_window_signals = 0L;

struct Window *gui_window = NULL;

struct Gadget *layout = NULL;
struct Gadget *timeout_slider = NULL, *timeout_integer = NULL;
struct Gadget *density_slider = NULL, *density_integer = NULL;
struct Gadget *speed_slider = NULL, *speed_integer = NULL;
struct Gadget *encoding_chooser = NULL;
struct Gadget *screenmode_requester = NULL;
struct Gadget *fog_checkbox = NULL, *wave_checkbox = NULL, *rotate_checkbox = NULL, *invert_checkbox = NULL;
struct List *encodings_list;

struct TagItem sl_2_int_map[] =
{
    {SLIDER_Level, INTEGER_Number},
    {TAG_DONE}
};

struct TagItem int_2_sl_map[] =
{
    {INTEGER_Number, SLIDER_Level},
    {TAG_DONE}
};

struct TagItem ica_targets[] =
{
    {ICA_TARGET, 0},
    {TAG_DONE}
};

char *encodings[] = {"Matrix", "DNA", "Binary", "Hexadecimal", "decimal", NULL };

BOOL window_open = FALSE;

static struct GLMatrixPrefs prefs;
static struct GLMatrixPrefs backup_prefs;
static BOOL Prefs_Backedup = FALSE;

extern ULONG csigflag;
extern struct Task *maintask;
extern BOOL do_blank;
extern BOOL blanked;

struct Hook* ScreenmodeHook = NULL;

BOOL ScreenmodeCallback(struct Hook* hook, VOID* object, ULONG modeid)
{
    W3D_Driver* found_driver = NULL;
    found_driver = IWarp3D->W3D_TestMode(modeid);
    if (found_driver != NULL)
    {
        return TRUE;
    }

    return FALSE;
}

BOOL gui_build_window(void)
{
    struct Node *node;
    int i;

    encodings_list = (struct List *)IExec->AllocVec(sizeof(struct List), MEMF_CLEAR);

    if (encodings_list != NULL)
    {
        IExec->NewList(encodings_list);
        for (i = 0; i < 5; i++)
        {
			node = (struct Node*)(IChooser->AllocChooserNode(CNA_Text, (ULONG)encodings[i], TAG_DONE));
            IExec->AddTail(encodings_list, node);
        }
    }

    ScreenmodeHook = IExec->AllocSysObjectTags(ASOT_HOOK, ASOHOOK_Entry, ScreenmodeCallback, TAG_DONE);

    layout = LayoutObject,
                LAYOUT_Orientation, LAYOUT_VERTICAL,
                LAYOUT_SpaceInner, TRUE,
                LAYOUT_SpaceOuter, TRUE,
                LAYOUT_DeferLayout, TRUE,

                /* Timeout Slider and Integer */
                LAYOUT_AddChild,
                LayoutObject,
                    LAYOUT_Orientation, LAYOUT_HORIZONTAL,
                    /* Timeout Slider */
                    LAYOUT_AddChild, timeout_slider =
                    SliderObject,
                        GA_RelVerify, TRUE,
                        GA_ID, Timeout,
                        SLIDER_Level, 300,
                        SLIDER_Min, 5,
                        SLIDER_Max, 1800,
                        SLIDER_Orientation, SLIDER_HORIZONTAL,
                        ICA_MAP, sl_2_int_map,
                    SliderEnd,

                    /* Timeout Integer */
                    LAYOUT_AddChild, timeout_integer =
                    IntegerObject,
                        GA_RelVerify, TRUE,
                        GA_ID, Timeout_Integer,
                        INTEGER_Number, 300,
                        INTEGER_Minimum, 5,
                        INTEGER_Maximum, 1800,
                        INTEGER_Arrows, FALSE,
                        ICA_MAP, int_2_sl_map,
                    IntegerEnd,
                LayoutEnd,

                CHILD_Label,
                LabelObject,
                    LABEL_Justification, LABEL_LEFT,
                    LABEL_Text, "Timeout: ",
                LabelEnd,

                LAYOUT_AddChild, SpaceObject, SpaceEnd,

                /* Density Slider and Integer */
                LAYOUT_AddChild,
                LayoutObject,
                    LAYOUT_Orientation, LAYOUT_HORIZONTAL,
                    /* Density Slider */
                    LAYOUT_AddChild, density_slider =
                    SliderObject,
                        GA_RelVerify, TRUE,
                        GA_ID, Density,
                        SLIDER_Level, 20,
                        SLIDER_Min, 2,
                        SLIDER_Max, 200,
                        SLIDER_Orientation, SLIDER_HORIZONTAL,
                        ICA_MAP, sl_2_int_map,
                    SliderEnd,

                    /* Density Integer */
                    LAYOUT_AddChild, density_integer =
                    IntegerObject,
                        GA_RelVerify, TRUE,
                        GA_ID, Density_Integer,
                        INTEGER_Number, 20,
                        INTEGER_Minimum, 2,
                        INTEGER_Maximum, 200,
                        INTEGER_Arrows, FALSE,
                        ICA_MAP, int_2_sl_map,
                    IntegerEnd,
                LayoutEnd,

                CHILD_Label,
                LabelObject,
                    LABEL_Justification, LABEL_LEFT,
                    LABEL_Text, "Density: ",
                LabelEnd,

                LAYOUT_AddChild, SpaceObject, SpaceEnd,

				/* Speed Slider and Integer */
                LAYOUT_AddChild,
                LayoutObject,
                    LAYOUT_Orientation, LAYOUT_HORIZONTAL,
                    /* Speed Slider */
                    LAYOUT_AddChild, speed_slider =
                    SliderObject,
                        GA_RelVerify, TRUE,
                        GA_ID, Speed,
                        SLIDER_Level, 100,
                        SLIDER_Min, 0,
                        SLIDER_Max, 300,
                        SLIDER_Orientation, SLIDER_HORIZONTAL,
                        ICA_MAP, sl_2_int_map,
                    SliderEnd,

                    /* Speed Integer */
                    LAYOUT_AddChild, speed_integer =
                    IntegerObject,
                        GA_RelVerify, TRUE,
                        GA_ID, Speed_Integer,
                        INTEGER_Number, 100,
                        INTEGER_Minimum, 0,
                        INTEGER_Maximum, 300,
                        INTEGER_Arrows, FALSE,
                        ICA_MAP, int_2_sl_map,
                    IntegerEnd,
                LayoutEnd,

                CHILD_Label,
                LabelObject,
                    LABEL_Justification, LABEL_LEFT,
                    LABEL_Text, "Speed: ",
                LabelEnd,

                LAYOUT_AddChild, SpaceObject, SpaceEnd,

                /* Encodings Chooser */
                LAYOUT_AddChild, encoding_chooser =
                ChooserObject,
                    GA_RelVerify, TRUE,
                    GA_ID, Encoding,
                    CHOOSER_PopUp, TRUE,
                    CHOOSER_Labels, encodings_list,
                ChooserEnd,

                CHILD_Label,
                LabelObject,
                    LABEL_Justification, LABEL_LEFT,
                    LABEL_Text, "Encoding: ",
                LabelEnd,

                LAYOUT_AddChild, SpaceObject, SpaceEnd,

                /* Screen mode requester */
                LAYOUT_AddChild, screenmode_requester =
                GetScreenModeObject,
                    GA_RelVerify, TRUE,
                    GA_ID, ScreenMode,
                    GETSCREENMODE_FilterFunc, ScreenmodeHook,
                GetScreenModeEnd,

                CHILD_Label,
                LabelObject,
                    LABEL_Justification, LABEL_LEFT,
                    LABEL_Text, "Screen Mode: ",
                LabelEnd,

                LAYOUT_AddChild, SpaceObject, SpaceEnd,

                /* Check boxes */
                LAYOUT_AddChild,
                LayoutObject,
                    LAYOUT_Orientation, LAYOUT_HORIZONTAL,
                    LAYOUT_HorizAlignment, LALIGN_CENTER,
                    LAYOUT_EvenSize, TRUE,

                    /* Fog */
                    LAYOUT_AddChild, fog_checkbox =
                    CheckBoxObject,
                        GA_ID, Fog,
                        GA_Text, "_Fog",
                        GA_Selected, TRUE,
                        GA_RelVerify, TRUE,
                    CheckBoxEnd,

                    /* Wave */
                    LAYOUT_AddChild, wave_checkbox =
                    CheckBoxObject,
                        GA_ID, Wave,
                        GA_Text, "_Wave",
                        GA_Selected, TRUE,
                        GA_RelVerify, TRUE,
                    CheckBoxEnd,

                    /* Rotate */
                    LAYOUT_AddChild, rotate_checkbox =
                    CheckBoxObject,
                        GA_ID, Rotate,
                        GA_Text, "_Rotate",
                        GA_Selected, TRUE,
                        GA_RelVerify, TRUE,
                    CheckBoxEnd,

                    /* Invert alpha */
                    LAYOUT_AddChild, invert_checkbox =
                    CheckBoxObject,
                        GA_ID, Invert,
                        GA_Text, "_Invert alpha",
                        GA_Selected, FALSE,
                        GA_RelVerify, TRUE,
                    CheckBoxEnd,

                LayoutEnd, /* End Check Boxes */


                LAYOUT_AddChild, SpaceObject, SpaceEnd,

                /* Buttons */
                LAYOUT_AddChild,
                LayoutObject,
                    LAYOUT_Orientation, LAYOUT_HORIZONTAL,
                    LAYOUT_HorizAlignment, LALIGN_CENTER,
                    LAYOUT_EvenSize, TRUE,

                    /* Save button */
                    LAYOUT_AddChild,
                    ButtonObject,
                        GA_ID, Save,
                        GA_Text, "_Save",
                        GA_RelVerify, TRUE,
                    ButtonEnd,

                    LAYOUT_AddChild, SpaceObject, SpaceEnd,

                    /* Use button */
                    LAYOUT_AddChild,
                    ButtonObject,
                        GA_ID, Use,
                        GA_Text, "_Use",
                        GA_RelVerify, TRUE,
                    ButtonEnd,

                    LAYOUT_AddChild, SpaceObject, SpaceEnd,

					/* Test button */
                    LAYOUT_AddChild,
                    ButtonObject,
						GA_ID, Test,
						GA_Text, "_Test",
                        GA_RelVerify, TRUE,
                    ButtonEnd,

                    LAYOUT_AddChild, SpaceObject, SpaceEnd,

                    /* Cancel button */
                    LAYOUT_AddChild,
                    ButtonObject,
                        GA_ID, Cancel,
                        GA_Text, "_Cancel",
                        GA_RelVerify, TRUE,
                    ButtonEnd,
                LayoutEnd, /* End Buttons */

             LayoutEnd; /* End Main Layout */

    if (layout != NULL)
    {
		ica_targets[0].ti_Data = (ULONG)timeout_integer;
		IIntuition->SetAttrsA(timeout_slider, ica_targets);
		ica_targets[0].ti_Data = (ULONG)timeout_slider;
		IIntuition->SetAttrsA(timeout_integer, ica_targets);

		ica_targets[0].ti_Data = (ULONG)density_integer;
		IIntuition->SetAttrsA(density_slider, ica_targets);
		ica_targets[0].ti_Data = (ULONG)density_slider;
		IIntuition->SetAttrsA(density_integer, ica_targets);

		ica_targets[0].ti_Data = (ULONG)speed_integer;
		IIntuition->SetAttrsA(speed_slider, ica_targets);
		ica_targets[0].ti_Data = (ULONG)speed_slider;
		IIntuition->SetAttrsA(speed_integer, ica_targets);

		gui_window_object = WindowObject,
		                        WA_Title, (ULONG)"GLMatrix",
                                WA_Width, 200,
                                WA_Height, 160,
                                WA_DragBar, TRUE,
                                WA_DepthGadget, TRUE,
                                WINDOW_ParentLayout, layout,
                                WINDOW_SharedPort, IDCMP_port,
                                WINDOW_AppPort, app_port,
                                WINDOW_Position, WPOS_CENTERSCREEN,
                                WindowEnd;

		if (gui_window_object != NULL)
		{
			return TRUE;
		}
	}

	return FALSE;
}


BOOL gui_init(void)
{
    IDCMP_port = IExec->CreateMsgPort();
    if (IDCMP_port != NULL)
    {
        app_port = IExec->CreateMsgPort();
        if (app_port != NULL)
        {
            if (gui_build_window())
            {
                glmatrix_prefs = read_prefs();
                if (glmatrix_prefs != NULL)
                {
                    IExec->CopyMem(glmatrix_prefs, &prefs, sizeof(prefs));
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

void free_chooser_list(struct List *list)
{
    struct Node *node, *nextnode;

    node = list->lh_Head;
	while ((nextnode = node->ln_Succ))
    {
        IChooser->FreeChooserNode(node);
        node = nextnode;
    }
    IExec->NewList(list);
}

void gui_destroy(void)
{
    IExec->FreeVec(glmatrix_prefs);

    if (gui_window_object != NULL)
    {
        IIntuition->DisposeObject(gui_window_object);
    }
    gui_window_object = NULL;

    if (ScreenmodeHook != NULL)
    {
        IExec->FreeSysObject(ASOT_HOOK, ScreenmodeHook);
    }

    if (encodings_list != NULL)
    {
        free_chooser_list(encodings_list);
        IExec->FreeVec(encodings_list);
        encodings_list = NULL;
    }

    if (app_port != NULL) IExec->DeleteMsgPort(app_port);
    app_port = NULL;

    if (IDCMP_port != NULL) IExec->DeleteMsgPort(IDCMP_port);
    IDCMP_port = NULL;
}

void set_attr(void)
{
	struct TagItem tags[] = { {0,0}, {TAG_DONE}};

    tags[0].ti_Tag  = INTEGER_Number;
    tags[0].ti_Data = prefs.glm_Timeout;
    IIntuition->SetGadgetAttrsA(timeout_integer, gui_window, NULL, tags);

    tags[0].ti_Data = prefs.glm_Density;
    IIntuition->SetGadgetAttrsA(density_integer, gui_window, NULL, tags);

    tags[0].ti_Data = prefs.glm_Speed;
    IIntuition->SetGadgetAttrsA(speed_integer, gui_window, NULL, tags);

    tags[0].ti_Tag  = SLIDER_Level;
    tags[0].ti_Data = prefs.glm_Timeout;
    IIntuition->SetGadgetAttrsA(timeout_slider, gui_window, NULL, tags);

    tags[0].ti_Data = prefs.glm_Density;
    IIntuition->SetGadgetAttrsA(density_slider, gui_window, NULL, tags);

    tags[0].ti_Data = prefs.glm_Speed;
    IIntuition->SetGadgetAttrsA(speed_slider, gui_window, NULL, tags);

    tags[0].ti_Tag  = CHOOSER_Selected;
    tags[0].ti_Data = prefs.glm_Encoding;
    IIntuition->SetGadgetAttrsA(encoding_chooser, gui_window, NULL, tags);

    tags[0].ti_Tag = GETSCREENMODE_DisplayID;
    tags[0].ti_Data = prefs.glm_ScreenModeID;
    IIntuition->SetGadgetAttrsA(screenmode_requester, gui_window, NULL, tags);

    tags[0].ti_Tag  = GA_Selected;
    tags[0].ti_Data = (ULONG)prefs.glm_Fog;
    IIntuition->SetGadgetAttrsA(fog_checkbox, gui_window, NULL, tags);

    tags[0].ti_Data = (ULONG)prefs.glm_Wave;
    IIntuition->SetGadgetAttrsA(wave_checkbox, gui_window, NULL, tags);

    tags[0].ti_Data = (ULONG)prefs.glm_Rotate;
    IIntuition->SetGadgetAttrsA(rotate_checkbox, gui_window, NULL, tags);

    tags[0].ti_Data = (ULONG)prefs.glm_Invert;
    IIntuition->SetGadgetAttrsA(invert_checkbox, gui_window, NULL, tags);
}

void gui_open(void)
{
    if (gui_window_object != NULL && !window_open)
    {
        set_attr();
        IIntuition->IDoMethod(gui_window_object, WM_OPEN, NULL);
        IIntuition->GetAttr(WINDOW_Window, gui_window_object, (ULONG *)&gui_window);
        IIntuition->GetAttr(WINDOW_SigMask, gui_window_object, &gui_window_signals);
        window_open = TRUE;
    }
}

void gui_close(void)
{
    if (gui_window_object != NULL && gui_window != NULL && window_open)
    {
        RA_CloseWindow(gui_window_object);
        IIntuition->GetAttr(WINDOW_Window, gui_window_object, (ULONG *)&gui_window);
        gui_window_signals = 0;
        window_open = FALSE;
    }
}

void gui_handle_window(void)
{
    ULONG code = 0;
    ULONG result = 0;

    if (gui_window_object != NULL)
    {
        result = RA_HandleInput(gui_window_object, &code);
        while(result != WMHI_LASTMSG)
        {
            switch (result & WMHI_CLASSMASK)
            {
                case WMHI_CLOSEWINDOW:
                {
                    gui_close();
                    break;
                } /* case CLOSEWINDOW */

                case WMHI_GADGETUP:
                {
                    ULONG attr = 0;

                    switch (RL_GADGETID(result))
                    {
                        case Timeout:
                        case Timeout_Integer:
                            IIntuition->GetAttr(INTEGER_Number, timeout_integer, &attr);
                            prefs.glm_Timeout = attr;
                            break;
                        case Density:
                        case Density_Integer:
                            IIntuition->GetAttr(INTEGER_Number, density_integer, &attr);
                            prefs.glm_Density = attr;
                            break;
                        case Speed:
                        case Speed_Integer:
                            IIntuition->GetAttr(INTEGER_Number, speed_integer, &attr);
                            prefs.glm_Speed = attr;
                            break;
                        case Encoding:
                            IIntuition->GetAttr(CHOOSER_Selected, encoding_chooser, &attr);
                            prefs.glm_Encoding = attr;
                            break;
                        case ScreenMode:
                            RequestScreenMode(screenmode_requester, gui_window);
                            IIntuition->GetAttr(GETSCREENMODE_DisplayID, screenmode_requester, &attr);
                            prefs.glm_ScreenModeID = attr;
                            break;
                        case Fog:
                            IIntuition->GetAttr(GA_Selected, fog_checkbox, &attr);
                            prefs.glm_Fog = (BOOL)attr;
                            break;
                        case Wave:
                            IIntuition->GetAttr(GA_Selected, wave_checkbox, &attr);
                            prefs.glm_Wave = (BOOL)attr;
                            break;
                        case Rotate:
                            IIntuition->GetAttr(GA_Selected, rotate_checkbox, &attr);
                            prefs.glm_Rotate = (BOOL)attr;
                            break;
                        case Invert:
                            IIntuition->GetAttr(GA_Selected, invert_checkbox, &attr);
                            prefs.glm_Invert = (BOOL)attr;
                            break;
                        case Save:
                            IExec->CopyMem(&prefs, glmatrix_prefs, sizeof(prefs));
                            save_prefs();
                            gui_close();
                            break;
                        case Use:
                            IExec->CopyMem(&prefs, glmatrix_prefs, sizeof(prefs));
                            use_prefs();
                            gui_close();
                            break;
						case Test:
							if (!Prefs_Backedup)
							{
								IExec->CopyMem(glmatrix_prefs, &backup_prefs, sizeof(prefs));
								Prefs_Backedup = TRUE;
							}
                            IExec->CopyMem(&prefs, glmatrix_prefs, sizeof(prefs));
							use_prefs();
			                blanked = TRUE;
			                do_blank = TRUE;
			                IExec->Signal(maintask, csigflag);
							break;
                        case Cancel:
							if (Prefs_Backedup)
							{
								IExec->CopyMem(&backup_prefs, glmatrix_prefs, sizeof(backup_prefs));
								Prefs_Backedup = FALSE;
								/* Resave the original prefs to ENV: */
								use_prefs();
							}
							IExec->CopyMem(glmatrix_prefs, &prefs, sizeof(prefs));
                            gui_close();
                            break;
                        default:
                            break;
                    } /* switch gadget */
                    break;
                } /* case GADGETUP */
            } /* switch result & CLASSMASK */

            result = RA_HandleInput(gui_window_object, &code);
        } /* result != LASTMSG */
    }
}

