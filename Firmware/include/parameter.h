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
sbit sensorStartPosi1 = P0^0; //初始位置传感器1
sbit sensorStartPosi2 = P0^1; //初始位置传感器2
sbit keyMotorForward = P0^2; //电机向前信号
sbit keyMotorBackward = P0^3; //电机向后信号
sbit electromagnetIn = P0^4; //电磁铁输入

//输出
sbit motorDirection = P1^0; //电机方向
sbit electromagnetOut = P1^1; //电磁铁输出
sbit motorPWM = P2^6; //电机脉冲

sbit testOut = P3^7;

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
extern unsigned char isStartPosition1; //初始位置
extern unsigned char isStartPosition2; //初始位置

extern unsigned char refreshDisplay; //刷新屏幕标志位 0 不刷新 1刷新

extern unsigned char initFlag; //初始化标志位
#endif
