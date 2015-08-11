#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "basefunc.h"

void ChangeScreenPage(WORD page);
void SendDataToScreen(WORD address, WORD dat);
void SendLongDataToScreen(WORD address, LONG dat);
#endif


