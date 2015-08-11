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

#include "basefunc.h"
#include "parameter.h"
#include "screen.h"
#include "uart.h"
#include <timer.h>
#include <key.h>
#include <dispatch.h>

#if 0
void PluseDelay(unsigned int a)
{
	while(a--);
}

void TestRun()
{
	/*
	驱动步进电机转动一圈
	步进电机参数：细分倍数:8	步数/圈(1.8/整步):1600
	实际的步进角=1.8/8=0.225，旋转一圈需要脉冲个数=360/0.225=1600
	*/
	unsigned int i = 0;
	secondMotorDirection = 0;
	for (i = 0; i < 1600; i++)
	{
		secondMotorPWM = 1;
		PluseDelay(800);
		secondMotorPWM = ~secondMotorPWM;
		PluseDelay(800);
	}
	while (1);
}
#endif

void main()
{
	WORD count10ms = 0;
	delay_ms(500);
	// 初始化硬件
	InitHardware();
    // 初始化参数
	InitParameters();
	
	while (1)
	{
		delay_us(100);
		//按键扫描---10us*30=300us=0.3ms,控制按键扫描频率
		count10ms++;
		if(count10ms > 29)
		{
			ScanKey();
			count10ms = 0;
		}

		AdjustTimerCount();
		CheckSensorInput();
		StartSecondMotor();
		DisplayParameters();
		SaveParameters();
	} 
}








