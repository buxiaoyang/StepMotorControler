/*------------------------------------------------------------------*/
/* --- STC MCU Limited ---------------------------------------------*/
/* --- STC12C5Axx Series 16-bit Timer Demo -------------------------*/
/* --- Mobile: (86)13922805190 -------------------------------------*/
/* --- Fax: 86-755-82905966 ----------------------------------------*/
/* --- Tel: 86-755-82948412 ----------------------------------------*/
/* --- Web: www.STCMCU.com -----------------------------------------*/
/* If you want to use the program or the program referenced in the  */
/* article, please specify in which data and procedures from STC    */
/*------------------------------------------------------------------*/

#ifndef __TIMER_H__
#define __TIMER_H__
#include "basefunc.h"

//extern unsigned int timer_count;
//extern unsigned int pulseSettingNumCount;
void timer_init();
void AdjustTimerCount();
void SetTimerParameter(WORD timerCount, WORD pulseCount);
void SetTimerParameter1(WORD timerCount, WORD pulseCount);

void StopPulseTimer();
#endif