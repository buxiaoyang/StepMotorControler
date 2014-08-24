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
		if(isStartPosition1 == 0)
		{
			refreshDisplay = 1;
		}
		isStartPosition1 = 1;
	}
	if(sensorStartPosi1 == 1)
	{
		if(isStartPosition1 == 1)
		{
			refreshDisplay = 1;
		}
		isStartPosition1 = 0;
	}
	//初始位置传感器2
	if(sensorStartPosi2 == 0)
	{
		if(isStartPosition2 == 0)
		{
			refreshDisplay = 1;
		}
		isStartPosition2 = 1;
	}
	if(sensorStartPosi2 == 1)
	{
		if(isStartPosition2 == 1)
		{
			refreshDisplay = 1;
		}
		isStartPosition2 = 0;
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
	uint count10ms = 0;
	unsigned long temp = 0;
	delay_ms(500);
	parameter_init();
	uart_init();
	timer_init();
	while(1)
	{
		delay_us(100);
		//按键扫描
		count10ms++;
		if(count10ms > 29)
		{
			Key_Scan();
			count10ms = 0;
		}
		//General the PWM
		if(pulseSettingNumCount > 20)
		{
			timer_count --;
		}
		else
		{
			timer_count ++;
		}
		if(timer_count < 1)
		{
			timer_count = 1;	
		}
		else if(timer_count > 50)
		{
			timer_count = 50;
		}
		//Get the sensor status
		getSensorStatus();
		if(isStartPosition2 || isStartPosition1)
		{
			isStartPosition = 1;
		}
		else if(!isStartPosition2 && !isStartPosition1)
		{
			isStartPosition = 0;
		}
		//初始化过程中判断原点
		if(initFlag == 1 && isStartPosition == 1)
		{
			pulseSettingNumCount = 4;
			currentPosition = 1;
			initFlag = 0;
		}
		//Save setting to eeprom
		if(saveSetting)
		{
			//计算电机旋转角
			temp = (unsigned long)pulseSettingNum*motorStepAngle;
			motorRotationAngle = temp/motorReducGearRatio;
			//计算丝杆导程
			temp = (unsigned long)pulseSettingNum*motorStepAngle;
			temp = 	(unsigned long)temp * screwPitch;
			temp = 	(unsigned long)temp / motorReducGearRatio;
			ballScrew = temp/3600;
			//保存设置
			ChangeScreenPage(0x04);
			parameter_save();
			ChangeScreenPage(0x02);
			saveSetting = 0;
			refreshDisplay = 1;
		}
		//刷新显示	
		if(refreshDisplay)
		{
			parameter_send_screen();
			refreshDisplay = 0;
		}
	}   
}








