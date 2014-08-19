#include <reg52.h>
#include <intrins.h>
#include <parameter.h>
#include <key.h>
#include <dispatch.h>
#include <timer.h>


void Key_Scan(void);

unsigned char Key_Scan_Steps = 0; 		//0：初始状态 如果有按键按下则进入1
										//1：延迟10ms
										//2：如果按键确实按下则进入3
										//3：确定按键
										//4：如果按键释放则进入0

void Key_Scan(void)
{
	testOut = ~testOut;
	switch(Key_Scan_Steps)
	{
		case 0:
			if(keyMotorForward == 0 || keyMotorBackward == 0)
			{
			   	Key_Scan_Steps = 1;
			}
		break;
		case 1:
			if(keyMotorForward == 0 || keyMotorBackward == 0)
			{
			   	Key_Scan_Steps = 2;
			}
			else
			{
				Key_Scan_Steps = 0;
			}
		break;
		case 2:
			if(keyMotorForward == 0) //电机前进
			{
				motorDirection = 1;
				pulseSettingNumCount = pulseSettingNum;
				currentPosition ++;
				timer_count = 50;
			}
			else if(keyMotorBackward == 0) //电机后退
			{
				motorDirection = 0;
				pulseSettingNumCount = pulseSettingNum;
				currentPosition --;
				timer_count = 50;
			}
			refreshDisplay = 1;
			Key_Scan_Steps = 3;
		break;
		case 3:
			if(keyMotorBackward == 1 && keyMotorForward == 1)
			{
			   	Key_Scan_Steps = 0;
			}
		break;
		default:
			 _nop_();
	}
}