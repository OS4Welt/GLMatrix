#include <exec/memory.h>
#include <intuition/intuition.h>
#include <prefs/prefhdr.h>
#include <Warp3D/Warp3D.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/iffparse.h>
#include <proto/intuition.h>
#include <proto/Warp3D.h>

#include "glmatrix_prefs.h"

struct GLMatrixPrefs *glmatrix_prefs = NULL;

struct GLMatrixPrefs* read_prefs(void)
{
	struct IFFHandle *iff_handle = NULL;
	struct ContextNode *cn = NULL;
	struct StoredProperty *sp = NULL;
    LONG version = 0;
    BOOL invalid_mode_id = FALSE;

	struct GLMatrixPrefs *prefs = IExec->AllocVec(sizeof(struct GLMatrixPrefs), MEMF_CLEAR | MEMF_ANY);

	if (prefs != NULL)
	{
		/* Loading default values into prefs */
		prefs->glm_Timeout = 300;
		prefs->glm_Density = 20;
		prefs->glm_Speed = 100;
		prefs->glm_Encoding = 0;
		prefs->glm_Fog = TRUE;
		prefs->glm_Wave = TRUE;
		prefs->glm_Rotate = TRUE;
		prefs->glm_Invert = FALSE;
		prefs->glm_ScreenModeID = 0;

		iff_handle = IIFFParse->AllocIFF();
		if (iff_handle != NULL)
		{
			iff_handle->iff_Stream = (ULONG)IDOS->Open(ENVPREFSFILE, MODE_OLDFILE);
			if (iff_handle->iff_Stream != 0)
			{
				IIFFParse->InitIFFasDOS(iff_handle);
				if (IIFFParse->OpenIFF(iff_handle, IFFF_READ) == 0)
				{
				    LONG error = 0;
                    W3D_Driver* found_driver = NULL;

					IIFFParse->PropChunk(iff_handle, ID_PREF, ID_PRHD);
					IIFFParse->StopChunk(iff_handle, ID_PREF, ID_GLMP);

					error = IIFFParse->ParseIFF(iff_handle, IFFPARSE_SCAN);

					sp = IIFFParse->FindProp(iff_handle, ID_PREF, ID_PRHD);
					if (sp != NULL)
					{
					    struct PrefHeader *head = sp->sp_Data;
					    version = head->ph_Version;
					} /* if sp */

					cn = IIFFParse->CurrentChunk(iff_handle);
					if (cn != NULL && cn->cn_ID == ID_GLMP)
					{
					    LONG read_length;
                        if (version > 0)
                        {
                            read_length = cn->cn_Size;
                        }
                        else
                        {
                            read_length = cn->cn_Size - (sizeof(BOOL) + sizeof(ULONG));
                        }
						IIFFParse->ReadChunkBytes(iff_handle, (APTR)prefs, read_length);

                        /* Test if the mode ID is compatible with W3D*/
                        found_driver = IWarp3D->W3D_TestMode(prefs->glm_ScreenModeID);
                        if (found_driver == NULL)
                            invalid_mode_id = TRUE;
					} /* if cn */

					IIFFParse->CloseIFF(iff_handle);
				} /* if OpenIFF */

				IDOS->Close((BPTR)iff_handle->iff_Stream);
			} /* if iff_stream */

			IIFFParse->FreeIFF(iff_handle);
		}/* if iff_handle */
	} /* if prefs */

    if (version < 2 || invalid_mode_id)
    {
        /* Old version of prefs file or invalid mode ID, write a new prefs file to ENV: and ENVARC: */
        W3D_ScreenMode* default_mode = IWarp3D->W3D_GetScreenmodeList();
		if (NULL != default_mode)
		{
			prefs->glm_ScreenModeID = default_mode->ModeID;
			IWarp3D->W3D_FreeScreenmodeList(default_mode);
			save_prefs();
		}
		else
		{
			/* No compatible screenmode was found. The user has W3D installed
			   but no supported graphics accelerator was found. */
			char* message = "W3D Was unable to find a compatible screen mode";
			struct EasyStruct ok = {
					sizeof(struct EasyStruct),
					0,
					"W3D Error",
					message,
					"OK"
			};

			IIntuition->EasyRequest(NULL, &ok, NULL);

			IExec->FreeVec(prefs);
			return NULL;
		}
    }
	
    return prefs;
}

void write_prefs(struct GLMatrixPrefs *prefs, char *filename)
{
	struct IFFHandle *iff_handle = NULL;

    struct PrefHeader header = {2,0,0};

    if(prefs != NULL)
    {
        iff_handle = IIFFParse->AllocIFF();
        if (iff_handle != NULL)
        {
            iff_handle->iff_Stream = (ULONG)IDOS->Open(filename, MODE_NEWFILE);
            if (iff_handle->iff_Stream != NULL)
            {
                IIFFParse->InitIFFasDOS(iff_handle);
                if (IIFFParse->OpenIFF(iff_handle, IFFF_RWBITS) == 0)
                {
                    IIFFParse->PushChunk(iff_handle, ID_PREF, ID_FORM, IFFSIZE_UNKNOWN);

                    /* Write Prefs header */
                    IIFFParse->PushChunk(iff_handle, ID_PREF, ID_PRHD, sizeof(struct PrefHeader));
                    IIFFParse->WriteChunkBytes(iff_handle, &header, sizeof(header));
                    IIFFParse->PopChunk(iff_handle);

                    /* Write GLMatrix prefs */
                    IIFFParse->PushChunk(iff_handle, ID_PREF, ID_GLMP, sizeof(struct GLMatrixPrefs));
                    IIFFParse->WriteChunkBytes(iff_handle, prefs, sizeof(*prefs));
                    IIFFParse->PopChunk(iff_handle);

                    IIFFParse->PopChunk(iff_handle);

                    IIFFParse->CloseIFF(iff_handle);
                }

                IDOS->Close((BPTR)iff_handle->iff_Stream);
            }
            IIFFParse->FreeIFF(iff_handle);
        }
    }
}

void use_prefs(void)
{
    write_prefs(glmatrix_prefs, ENVPREFSFILE);
}

void save_prefs(void)
{
    write_prefs(glmatrix_prefs, ENVARCPREFSFILE);
    write_prefs(glmatrix_prefs, ENVPREFSFILE);
}
