/***************************************************************************/
// 程序：清洗设备控制器
// 模块：主函数	
// 文件：main.c
// 作者：卜晓旸
// 版本：V0.02
// 日期：2013年11月8日
// 功能：主函数
// 芯片：STC12C5A60S2
// 编译：Keil uVision3 V3.90
/***************************************************************************/

#include <reg52.h>
#include <basefunc.h>
#include <parameter.h>
#include <uart.h>
#include <timer.h>
#include <key.h>
#include <dispatch.h>

void parameter_send_screen()
{
	SendDataToScreen(0x0010, currentPosition);
	SendDataToScreen(0x0000, pulseSettingNum);
	SendDataToScreen(0x0002, pulseSettingFreq);
	SendDataToScreen(0x0004, motorStepAngle);
	SendDataToScreen(0x0006, screwPitch);
	SendDataToScreen(0x0008, motorReducGearRatio);
	SendDataToScreen(0x000A, ballScrew);
	SendDataToScreen(0x000C, motorRotationAngle);
	SendDataToScreen(0x0012, isStartPosition);
}

void getSensorStatus()
{
	//初始位置传感器
	if(sensorStartPosi1 == 0)
	{
		if(isStartPosition == 0)
		{
			refreshDisplay = 1;
		}
		isStartPosition = 1;
	}
	if(sensorStartPosi1 == 1)
	{
		if(isStartPosition == 1)
		{
			refreshDisplay = 1;
		}
		isStartPosition = 0;
	}
	//电磁铁输入
	if(electromagnetIn == 0)
	{
		electromagnetOut = 0;
	}
	else
	{
		electromagnetOut = 1;
	}
}

/***************************************************************************/
// 主函数
// 参数：无
// 返回值：无	
/***************************************************************************/
void main()
{
	delay_ms(500);
	parameter_init();
	uart_init();
	timer_init();
	while(1)
	{
		delay_us(100);
		if(pulseSettingNumCount > 150)
		{
			timer_count --;
		}
		else
		{
			timer_count ++;
		}
		if(timer_count < 12)
		{
			timer_count = 12;	
		}
		else if(timer_count > 50)
		{
			timer_count = 50;
		}
		getSensorStatus();
		if(saveSetting)
		{
			ChangeScreenPage(0x04);
			parameter_save();
			ChangeScreenPage(0x02);
			saveSetting = 0;
		}
		//刷新显示	
		if(refreshDisplay)
		{
			parameter_send_screen();
			refreshDisplay = 0;
		}
	}   
}








