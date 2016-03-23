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

#include "basefunc.h"

/***************************************************************************/
// 引脚定义
/***************************************************************************/
//输入
sbit sensorStartPosi1 = P0^0; //初始位置传感器1
sbit sensorStartPosi2 = P0^1; //初始位置传感器2
sbit keyMotorForward  = P0^2; //电机向前信号
sbit keyMotorBackward = P0^3; //电机向后信号
sbit electromagnetIn  = P0^4; //电磁铁输入

// added by leitiannet
sbit keyTestMotor     = P0^5; //电机测试信号
sbit keyInitMotor     = P0^6; //电机初始化信息
sbit keyUserControl   = P0^7; //等待用户输入，第二个电机才开始运转

sbit secondMotorFlag  = P1^7; //第二个电机开始反转指示灯
sbit userControlFlag  = P1^6; //

sbit sensorStartFlag  = P1^4; //输出低电平给予用户反馈

// end

//输出
sbit motorDirection = P1^0;   //电机方向
sbit electromagnetOut = P1^1; //电磁铁输出
sbit motorPWM = P2^6;         //电机脉冲

// TODO:定义第二个电机接口
sbit secondMotorDirection = P2^0;   //电机方向
sbit secondMotorPWM = P2^1;         //电机脉冲

sbit testOut = P3^7;

#if 0
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

extern BOOL refreshDisplay; //刷新屏幕标志位 0 不刷新 1刷新
extern BOOL saveSetting;
extern unsigned char initFlag; //初始化标志位
#endif

// =============added by leitiannet=============

// 预定义最多40个旋转角
#define ROTATIONANGLE_SIZE			40

// 定义旋转角在触摸屏中对应的地址
// 注意:最后一个字必须以0开始(便于与数组下标对应):0x0100-0x014E
#define SCREEN_ROTATIONANGLE_BEGIN	0x0100
#define SCREEN_ROTATIONANGLE_END	SCREEN_ROTATIONANGLE_BEGIN + (ROTATIONANGLE_SIZE * 2)

// 电机标识
typedef enum
{
	MOTORFLAG_UNKOWN = 0,
	MOTORFLAG_ONE,
	MOTORFLAG_TWO,
	MOTORFLAG_ALL
} MotorFlag;

// 电机位置
typedef enum
{
	MOTORPOSITION_BEGIN = 1,
	MOTORPOSITION_END	= ROTATIONANGLE_SIZE
} MotorPosition;

// 电机动作
typedef enum
{
	MOTOR_UNKOWN = 0,
	MOTOR_INIT,
	MOTORDIRECTION_FORWARD,
	MOTORDIRECTION_BACKWARD,
} MotorAction;

// 
typedef enum
{
	BUTTON_UNKOWN = 0,
	BUTTON_INIT,
	BUTTON_FORWARD,
	BUTTON_BACKWARD,
} ButtonAction;


void EnableRefreshDisplay();
void EnableSaveSetting();
void DisableRefreshDisplay();
void DisableSaveSetting();
BOOL IsRefreshDisplay();
BOOL IsSaveSetting();
BOOL IsInitPosition();

// 
void InitHardware();
void GetSensorStatus();
void CheckSensorInput();

// 控制电机
void DisablePulse();
void ChangePulse();
void FinishPulse();
void HandleMotor(int addr, int dat);
void InitMotor();
void ControlMotor(int direction);
void StartSecondMotor();

void TestMotor();

void UserControl();

// 设置参数---key为变量对应的地址
void ResetParameters();
void InitParameters();
void SetParameter(int key, int dat);
void SaveParameters();
void DisplayParameters();
void UpdateParameters();
void SetRotationAngle(int key, int dat);
void SyncRotationAngleNumber();
void SyncRotationAngle();

// 解析串口发送过来的命令
void ParseCommand(unsigned char* buf, int len);
void HandleReturn(int addr, int dat);
void HandleButtuon(int addr, int dat);

BOOL ReadParametersFromEEPROM();
BOOL SaveParametersToEEPROM();


void SaveSnapshot();
void RestoreSnapshot();

void SetCurrentPage(int page);
// end

#endif
