/***************************************************************************/
// 程序：清洗设备控制器
// 模块：参数头文件
// 文件：parameter.h
// 作者：卜晓旸
// 版本：V0.02
// 日期：2013年11月8日
// 功能：参数头文件
// 芯片：STC12C5A60S2
// 编译：Keil uVision3 V3.90
/***************************************************************************/

#ifndef __PARAMETER_H__
#define __PARAMETER_H__

#include <reg52.h>

/***************************************************************************/
// 引脚定义
/***************************************************************************/
//输入
sbit sensorIn1 = P0^0; //传感器1
sbit sensorIn2 = P0^1; //传感器2
sbit sensorIn3 = P0^2; //传感器3
sbit sensorIn4 = P0^3; //传感器4
sbit sensorIn5 = P0^4; //传感器5
sbit sensorIn6 = P0^5; //传感器6
sbit sensorIn7 = P0^6; //传感器7
sbit sensorIn8 = P0^7; //传感器8
sbit sensorIn9 = P2^0; //传感器9
sbit sensorIn10 = P2^1; //传感器10

sbit KeyStart = P2^2; //开始按钮
sbit KeyStop = P2^3; //急停按钮

//输出
sbit cylinderOut1 = P1^0; //气缸1
sbit cylinderOut2 = P1^1; //气缸2
sbit cylinderOut3 = P2^4; //气缸3
sbit cylinderOut4 = P2^5; //气缸4
sbit cylinderOut5 = P1^4; //气缸5
sbit montorOut = P1^5;		//电机输出
sbit systemAlarmOut = P1^6; //系统警报

sbit TestOut = P2^6;

/***************************************************************************/
// 参数声明
/***************************************************************************/
void parameter_init();
unsigned char parameter_save();

extern unsigned char currentPosition; //当前位置 1~20
extern unsigned int pulseSettingNum; //脉冲个数
extern unsigned int pulseSettingFreq; //脉冲频率
extern unsigned int motorStepAngle; //电机步进角
extern unsigned int screwPitch; //丝杆丝距
extern unsigned int motorReducGearRatio; //电机减速比
extern unsigned int ballScrew; //丝杆导程
extern unsigned int motorRotationAngle; //电机旋转角
extern unsigned char isStartPosition; //初始位置

extern unsigned char refreshDisplay; //刷新屏幕标志位 0 不刷新 1刷新

#endif
