#ifndef _AUTORESUMEINFO_H
#define _AUTORESUMEINFO_H

#include "gfxSettings.h"

C_DECLARATIONS_BEGIN

void saveAutoResumeInfoCryptic(void);
void saveAutoResumeInfoToRegistry(void);
int getAutoResumeInfoCryptic( void );
int getAutoResumeInfoFromRegistry( GfxSettings * gfxSettings, char * accountName, int * dontSaveName );

C_DECLARATIONS_END

#endif