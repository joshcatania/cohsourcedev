#ifndef _SEQSKELETON_H
#define _SEQSKELETON_H

#include "seq/seq.h"
#ifdef CLIENT
#include "graphics/FX/fxlists.h"
#endif

void animSetHeader(SeqInst *seq, int preserveOldAnimation );
int animCheckForLoadingObjects(GfxNode* node, FxHandle seqHandle);
void animCalcObjAndBoneUse(GfxNode* pNode, FxHandle seqHandle);


#endif
