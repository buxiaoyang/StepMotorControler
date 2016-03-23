#include "key.h"
#include <intrins.h>
#include <parameter.h>
#include <key.h>
#include <dispatch.h>
#include <timer.h>
#include <basefunc.h>


#define STATE_MACHINE_DELAY

static unsigned char Key_Scan_Steps = 0; 		//0：初始状态 如果有按键按下则进入1
												//1：延迟10ms
												//2：如果按键确实按下则进入3
												//3：确定按键
												//4：如果按键释放则进入0


static void OnKeyPressed()
{
	if (keyMotorForward == 0) //电机前进
	{
		ControlMotor(MOTORDIRECTION_FORWARD);
	}
	else if (keyMotorBackward == 0) //电机后退
	{
		ControlMotor(MOTORDIRECTION_BACKWARD);	
	}
	else if (keyTestMotor == 0)
	{
		TestMotor();
	}
	else if (keyInitMotor == 0)
	{
		InitMotor();
	}
	else if (keyUserControl == 0)
	{
		UserControl();
	}
}

void ScanKey(void)
{
	//testOut = ~testOut;
	switch(Key_Scan_Steps)
	{
		case 0:
			if (keyMotorForward == 0 || keyMotorBackward == 0 ||
			    keyTestMotor == 0 || keyInitMotor == 0 || keyUserControl == 0)
			{
			   	Key_Scan_Steps = 1;
			}
			break;
			
		case 1:
			#ifdef STATE_MACHINE_DELAY
				// 主循环中不延迟，所以这里要延迟20ms进行消除抖动
				delay_ms(20);
			#endif
			
			if (keyMotorForward == 0 || keyMotorBackward == 0 ||
			    keyTestMotor == 0 || keyInitMotor == 0  || keyUserControl == 0)
			{
			   	Key_Scan_Steps = 2;
			}
			else
			{
				Key_Scan_Steps = 0;
			}
			break;
			
		case 2:
			OnKeyPressed();
			Key_Scan_Steps = 3;
			break;
			
		case 3:
			if (keyMotorBackward == 1 && keyMotorForward == 1 && 
			    keyTestMotor == 1 && keyInitMotor == 1 && keyUserControl == 1)
			{
			   	Key_Scan_Steps = 0;
			}
			break;
			
		default:
			 _nop_();
	}
}


void TimerScanKey(void)
{
	switch(Key_Scan_Steps)
	{
		case 0:
			if (keyMotorForward == 0 || keyMotorBackward == 0 ||
			    keyTestMotor == 0 || keyInitMotor == 0 || keyUserControl == 0)
			{
			   	Key_Scan_Steps = 1;
			}
			break;
			
		case 1:
			if (keyMotorForward == 0 || keyMotorBackward == 0 ||
			    keyTestMotor == 0 || keyInitMotor == 0  || keyUserControl == 0)
			{
				OnKeyPressed();
				Key_Scan_Steps = 2;
			}
			else
			{
				Key_Scan_Steps = 0;
			}
			break;
			
		case 2:
			if (keyMotorBackward == 1 && keyMotorForward == 1 && 
			    keyTestMotor == 1 && keyInitMotor == 1 && keyUserControl == 1)
			{
			   	Key_Scan_Steps = 0;
			}
			break;
			
		default:
			 _nop_();
	}
}

void MainScanKey(void)
{
	unsigned char flag = 0;
	if (keyMotorForward == 0 || keyMotorBackward == 0 ||
		keyTestMotor == 0 || keyInitMotor == 0 || keyUserControl == 0)
	{
		delay_ms(20);
		if (keyMotorForward == 0 || keyMotorBackward == 0 ||
			keyTestMotor == 0 || keyInitMotor == 0 || keyUserControl == 0)
		{
			/*
			while (keyMotorBackward == 1 && keyMotorForward == 1 && 
			    keyTestMotor == 1 && keyInitMotor == 1 && keyUserControl == 1);
			*/
			flag = 1;
		}
	}

	if (flag == 1)
	{
		// 执行相关操作
		OnKeyPressed();
		flag = 0;
	}
		
}

