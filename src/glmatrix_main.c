#include <exec/types.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <libraries/commodities.h>
#include <workbench/startup.h>
#include <proto/exec.h>
#include <proto/commodities.h>
#include <proto/intuition.h>

#include "glmatrix_gui.h"
#include "glmatrix_prefs.h"

extern BOOL screenblanker_init(void);
extern void screenblanker_destroy(void);
extern void screenblanker_blank(void);
extern void screenblanker_unblank(void);
extern void screenblanker_display(void);

const char* VersionTag = "$VER: GLMatrix 1.3 (24.07.05)";

/* These are not in libauto.a */
struct Library *CxBase = NULL;
struct CommoditiesIFace *ICommodities = NULL;

CxObj *broker = NULL;
CxObj *screensaver_object = NULL;
struct MsgPort *broker_mp = NULL;
ULONG cxsigflag = 0;

LONG csignal = 0L;
ULONG csigflag = 0L;
struct Task *maintask = NULL;

static ULONG timeout = 0;
BOOL do_blank = FALSE;
BOOL blanked = FALSE;

struct NewBroker newbroker = {
    NB_VERSION,
    "GLMatrix",
    "GLMatrix",
    "MiniGL Matrix Screen Saver",
    NBU_UNIQUE | NBU_NOTIFY,
    COF_SHOW_HIDE,
    0,
    0,
    0
};

void handle_signal(void)
{
    if (do_blank)
    {
        screenblanker_blank();
        do_blank = FALSE;
    }
    else
    {
        screenblanker_unblank();
    }
}

void ProcessMessage(void)
{
    ULONG signal = 0L;
    BOOL running = TRUE;
    CxMsg *msg = NULL;

    while (running)
    {
        if (blanked)
        {
            screenblanker_display();

            /* Check and clear signals */
            signal = IExec->SetSignal(0L, cxsigflag | csigflag | SIGBREAKF_CTRL_C);
        }
        else
        {
            signal = IExec->Wait(cxsigflag | csigflag | SIGBREAKF_CTRL_C | gui_window_signals);
        }

        if (signal & gui_window_signals)
        {
            gui_handle_window();
        }

        if (signal & cxsigflag)
        {
            while ((msg = (CxMsg *)IExec->GetMsg(broker_mp)))
            {
                ULONG msgtype = ICommodities->CxMsgType(msg);
                ULONG msgid = ICommodities->CxMsgID(msg);

                switch (msgtype)
                {
                    case CXM_IEVENT:
                        break;
                    case CXM_COMMAND:
                        switch (msgid)
                        {
                            case CXCMD_DISABLE:
                                ICommodities->ActivateCxObj(broker, FALSE);
                                break;
                            case CXCMD_ENABLE:
                                ICommodities->ActivateCxObj(broker, TRUE);
                                break;

                            case CXCMD_APPEAR:
                                gui_open();
                                break;
                            case CXCMD_DISAPPEAR:
                                gui_close();
                                break;

                            case CXCMD_UNIQUE:
                                gui_open();
                                break;

                            case CXCMD_KILL:
                                running = FALSE;
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }

                IExec->ReplyMsg((struct Message *)msg);
            } /* while messages */
        }  /*if message signal */

        if (signal & SIGBREAKF_CTRL_C)
        {
            running = FALSE;
        }
        else if (signal & csigflag)
        {
            handle_signal();
        }
    }
}

void screensaver(CxMsg *cxm, CxObj *co)
{
	struct InputEvent *ie = (struct InputEvent*)(ICommodities->CxMsgData(cxm));

    if (ie->ie_Class == IECLASS_TIMER)
    {
        if (!blanked)
        {
            if (++timeout >= (glmatrix_prefs->glm_Timeout * 10L))
            {
                /* blank */
                blanked = TRUE;
                do_blank = TRUE;
                IExec->Signal(maintask, csigflag);
            }
        }
    }
    else if (ie->ie_Class != IECLASS_TIMER)
    {
        /* unblank */
        timeout = 0L;
        if (blanked)
        {
            do_blank = FALSE;
            IExec->Signal(maintask, csigflag);
            blanked = FALSE;
        }
    }
}

BOOL open_libs(void)
{
    CxBase = IExec->OpenLibrary("commodities.library", 37L);
    if (CxBase == NULL) return FALSE;

	ICommodities = (struct CommoditiesIFace*)IExec->GetInterface(CxBase, "main", 1, NULL);
	if (ICommodities == NULL) return FALSE;

    return TRUE;
}

void close_libs(void)
{
	if (ICommodities != NULL) IExec->DropInterface((struct Interface*)ICommodities);

    if (CxBase != NULL) IExec->CloseLibrary(CxBase);
    CxBase = NULL;
}

int main(void)
{
    CxMsg *msg = NULL;

    if (open_libs())
    {
		if (!screenblanker_init())
		{
			char* message = "MiniGL failed to initialize\nGLMatrix requiers Warp3D and a compatible graphics accelerator";
			struct EasyStruct ok = {
					sizeof(struct EasyStruct),
					0,
					"MiniGL init failed",
					message,
					"OK"
			};

			IIntuition->EasyRequest(NULL, &ok, NULL);
			return 0;
		}

        if (gui_init())
        {
            maintask = IExec->FindTask(NULL);
            csignal = IExec->AllocSignal(-1L);
            csigflag = 1L << csignal;

            broker_mp = IExec->CreateMsgPort();
            if (broker_mp != NULL)
            {
                newbroker.nb_Port = broker_mp;
                broker = ICommodities->CxBroker(&newbroker, NULL);
                if (broker != NULL)
                {
                    cxsigflag = 1L << broker_mp->mp_SigBit;

                    screensaver_object = CxCustom(screensaver, 0L);
                    if(screensaver_object != NULL)
                    {
                        ICommodities->AttachCxObj(broker, screensaver_object);

                        if (!ICommodities->CxObjError(screensaver_object))
                        {
                            ICommodities->ActivateCxObj(broker, 1L);

							IExec->SetTaskPri(IExec->FindTask(NULL), -10);

                            ProcessMessage();
                        }
                    }

                    ICommodities->DeleteCxObjAll(broker);
					while ((msg = (CxMsg *)IExec->GetMsg(broker_mp)))
                    {
                        IExec->ReplyMsg((struct Message *)msg);
                    }
                }

                IExec->DeletePort(broker_mp);
            }
            IExec->FreeSignal(csignal);
        }
        screenblanker_destroy();
    }

    gui_destroy();
    close_libs();
    return 0;
}

void wbmain(struct WBStartup *args)
{
    main();
}

