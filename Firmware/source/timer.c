#include "timer.h"
#include <intrins.h>
#include <parameter.h>
#include <key.h>

#include "debug.h"

//-----------------------------------------------

/* define constants */
#define FOSC 11059200L
#define MODE1T                      //Timer clock mode, comment this line is 12T mode, uncomment is 1T mode

#ifdef MODE1T
#define T1MS (65536-FOSC/1000)      //1ms timer calculation method in 1T mode
#else
#define T1MS (65536-FOSC/12/1000)   //1ms timer calculation method in 12T mode
#endif

/* define SFR */
//sfr AUXR = 0x8e;                    //Auxiliary register

/* define variables */
static WORD count;                         //1000 times counter
WORD timer_count;
WORD pulseSettingNumCount;
//-----------------------------------------------

/* Timer0 interrupt routine */
void tm0_isr() interrupt 1 using 1
{
    //TL0 = T1MS;                     //reload timer0 low byte
    //TH0 = T1MS >> 8;                //reload timer0 high byte
	TL0 = 0x8B;		//设置定时初值
	TH0 = 0xFC;		//设置定时初值
    if (count-- == 0)               //1ms * 1000 -> 1s
    {
		if (pulseSettingNumCount > 0)
		{
			//DebugParameter("pulseSettingNumCount", pulseSettingNumCount);
			//motorPWM = 1;
			//pulseSettingNumCount --;
			//motorPWM = ~motorPWM;
			DisablePulse();
			pulseSettingNumCount--;
			ChangePulse();
		}
		else
		{  
			FinishPulse();
		}
		count = timer_count;               //reset counter
    }
}

//-----------------------------------------------

/* main program */
void timer_init()
{
#ifdef MODE1T
    AUXR |= 0x80;                    //timer0 work in 1T mode
#endif
    TMOD = 0x01;                    //set timer0 as mode1 (16-bit)
    //TL0 = T1MS;                     //initial timer0 low byte
    //TH0 = T1MS >> 8;                //initial timer0 high byte
	TL0 = 0x8B;		//设置定时初值
	TH0 = 0xFC;		//设置定时初值
    TR0 = 1;                        //timer0 start running
    ET0 = 1;                        //enable timer0 interrupt
    EA = 1;                         //open global interrupt switch
    count = 0;                      //initial counter
	timer_count = 50;
	pulseSettingNumCount = 0;
}

void AdjustTimerCount()
{
	if (pulseSettingNumCount > 20)
	{
		timer_count --;
	}
	else
	{
		timer_count ++;
	}
	
	if (timer_count < 1)
	{
		timer_count = 1;	
	}
	else if (timer_count > 50)
	{
		timer_count = 50;
	}
}

void StopPulseTimer()
{
	TR0 = 0;
    ET0 = 0; 
	FinishPulse();
	Debug("StopPulseTimer\r\n");
}

void SetTimerParameter(WORD timerCount, WORD pulseCount)
{
	TR0 = 0;
    ET0 = 0; 
    
	//DebugParameter("before timerCount", timerCount);
	//DebugParameter("before pulseSettingNumCount", pulseCount);
	if (timerCount > 0)
	{
		timer_count = timerCount;
	}

	if (pulseCount > 0)
	{
		pulseSettingNumCount = pulseCount;
	}
	
	
	TR0 = 1;
    ET0 = 1;   
	//DebugParameter("timer_count", timer_count);
	//DebugParameter("pulseSettingNumCount", pulseSettingNumCount);	
}
