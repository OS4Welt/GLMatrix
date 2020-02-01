#include "glmatrix.h"
#include "glmatrix_prefs.h"
#include "yarandom.h"

int width;
int height;

static BOOL blanked = FALSE;

BOOL screenblanker_init(void)
{
	if (MGLInit())
	{
		mglChooseWindowMode(FALSE);
		ya_rand_init(0);
		return TRUE;
	}

	return FALSE;
}

void screenblanker_destroy(void)
{
    MGLTerm();
}

void screenblanker_display(void)
{
	if (blanked)
	{
		mglLockDisplay();

		draw_matrix(mps);
		mglUnlockDisplay();
		mglSwitchDisplay();
	}
}

void screenblanker_blank(void)
{
    if (!blanked)
    {
        if (mglCreateContextFromID(glmatrix_prefs->glm_ScreenModeID, &width, &height))
        {
            mglLockMode(MGL_LOCK_MANUAL);
			mglEnableSync(GL_TRUE);
			init_matrix();
			blanked = TRUE;
        }
    }
}

void screenblanker_unblank(void)
{
    if (blanked)
    {
	    blanked = FALSE;
        mglDeleteContext();
    }
}
