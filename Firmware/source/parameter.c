/***************************************************************************/
// 程序：清洗设备控制器
// 模块：参数
// 文件：parameter.c
// 作者：卜晓旸
// 版本：V0.02
// 日期：2013年11月8日
// 功能：参数
// 芯片：STC12C5A60S2
// 编译：Keil uVision3 V3.90
/***************************************************************************/

#include <parameter.h>
#include <eeprom.h>
#include <basefunc.h>

/***************************************************************************/
// 参数定义	
/***************************************************************************/
unsigned char currentPosition; //当前位置 1~20
unsigned int pulseSettingNum; //脉冲个数
unsigned int pulseSettingFreq; //脉冲频率
unsigned int motorStepAngle; //电机步进角
unsigned int screwPitch; //丝杆丝距
unsigned int motorReducGearRatio; //电机减速比
unsigned int ballScrew; //丝杆导程
unsigned int motorRotationAngle; //电机旋转角
unsigned char isStartPosition; //初始位置
unsigned char isStartPosition1; //初始位置
unsigned char isStartPosition2; //初始位置

unsigned char refreshDisplay; //刷新屏幕标志位 0 不刷新 1刷新

unsigned char initFlag; //初始化标志位

/***************************************************************************/
// 读取参数
// 参数：无
// 返回值：0失败 1成功	
/***************************************************************************/
unsigned char parameter_read()
{
	
	unsigned char result = 1;
	delay_ms(10); 
	if(IapReadByte(IAP_ADDRESS+200) == 0xEE)
	{	
		pulseSettingNum = ((IapReadByte(IAP_ADDRESS+0) << 8) | IapReadByte(IAP_ADDRESS+1));
		//pulseSettingFreq = ((IapReadByte(IAP_ADDRESS+2) << 8) | IapReadByte(IAP_ADDRESS+3));
		//motorStepAngle = ((IapReadByte(IAP_ADDRESS+4) << 8) | IapReadByte(IAP_ADDRESS+5));
		screwPitch = ((IapReadByte(IAP_ADDRESS+6) << 8) | IapReadByte(IAP_ADDRESS+7));
		motorReducGearRatio = ((IapReadByte(IAP_ADDRESS+8) << 8) | IapReadByte(IAP_ADDRESS+9));
		ballScrew = ((IapReadByte(IAP_ADDRESS+10) << 8) | IapReadByte(IAP_ADDRESS+11));
		motorRotationAngle = ((IapReadByte(IAP_ADDRESS+12) << 8) | IapReadByte(IAP_ADDRESS+13));
		result = 1;
	}
	else
	{
		pulseSettingNum = 1600;
		screwPitch = 12;
		motorReducGearRatio = 1;
		ballScrew = 12;
		motorRotationAngle = 360;
		result = 0;
	}
	pulseSettingFreq = 6070;
	motorStepAngle = 225;
	return result;
}

/***************************************************************************/
// 参数初始化函数
// 参数：无
// 返回值：无	
/***************************************************************************/
void parameter_init()
{
	currentPosition = 1; //当前位置 1~20
	if(!parameter_read())
	{
		pulseSettingNum = 400; //脉冲个数
		pulseSettingFreq = 400; //脉冲频率
		motorStepAngle = 180; //电机步进角
		screwPitch = 200; //丝杆丝距
		motorReducGearRatio = 100; //电机减速比
		ballScrew = 200; //丝杆导程
		motorRotationAngle = 36000; //电机旋转角
	}
	isStartPosition = 0; //初始位置
	refreshDisplay = 1;
	initFlag = 0;
}

/***************************************************************************/
// 参数保存到eeprom
// 参数：无
// 返回值：0失败 1成功	
/***************************************************************************/
unsigned char parameter_save()
{
	
	unsigned char result = 1;
    delay_ms(10);                      //Delay
	IapEraseSector(IAP_ADDRESS); //擦除EEPROM
   	IapProgramByte(IAP_ADDRESS+0, (BYTE)(pulseSettingNum>>8));
	IapProgramByte(IAP_ADDRESS+1, (BYTE)pulseSettingNum);
	IapProgramByte(IAP_ADDRESS+2, (BYTE)(pulseSettingFreq>>8));
	IapProgramByte(IAP_ADDRESS+3, (BYTE)pulseSettingFreq);
	IapProgramByte(IAP_ADDRESS+4, (BYTE)(motorStepAngle>>8));
	IapProgramByte(IAP_ADDRESS+5, (BYTE)motorStepAngle);
	IapProgramByte(IAP_ADDRESS+6, (BYTE)(screwPitch>>8));
	IapProgramByte(IAP_ADDRESS+7, (BYTE)screwPitch);
	IapProgramByte(IAP_ADDRESS+8, (BYTE)(motorReducGearRatio>>8));
	IapProgramByte(IAP_ADDRESS+9, (BYTE)motorReducGearRatio);
	IapProgramByte(IAP_ADDRESS+10, (BYTE)(ballScrew>>8));
	IapProgramByte(IAP_ADDRESS+11, (BYTE)ballScrew);
	IapProgramByte(IAP_ADDRESS+12, (BYTE)(motorRotationAngle>>8));
	IapProgramByte(IAP_ADDRESS+13, (BYTE)motorRotationAngle);
	IapProgramByte(IAP_ADDRESS+200, 0xEE); //写入标志位
	refreshDisplay = 0;
	return result;
}


