#include "parameter.h"
#include "screen.h"
#include "eeprom.h"
#include "timer.h"
#include "uart.h"
#include "debug.h"

#define REVERSE_DIRECTION

// 第二电机运行是否需要由用户控制
//#define USER_CONTROL 

//TODO:使用结构体封装
/***************************************************************************/
// 参数定义	
/***************************************************************************/
unsigned char currentPosition;  //当前位置 1~20
unsigned int  pulseSettingNum;  //脉冲个数
unsigned int  pulseSettingFreq; //脉冲频率
unsigned int  motorStepAngle;   //电机步进角
unsigned int  screwPitch;       //丝杆丝距
unsigned int  motorReducGearRatio; //电机减速比

unsigned long  ballScrew;          //丝杆导程
unsigned long  motorRotationAngle; //电机旋转角
unsigned char isStartPosition;     //初始位置
unsigned char isStartPosition1;    //初始位置
unsigned char isStartPosition2;    //初始位置

// unused
unsigned char initFlag = 0; //初始化标志位

BOOL refreshDisplay = FALSE; //刷新屏幕标志位 0 不刷新 1刷新
BOOL saveSetting = FALSE; // 保存参数标志位

static BOOL inputChanged = FALSE;
static int  motorAction = MOTOR_UNKOWN;
static unsigned int rotationAngles[ROTATIONANGLE_SIZE] = {0};
static int rotationAngleNumber = 0;
static int currentRotationAngleIndex = -1;

// 用于支持另外一个电机
static BOOL startSecondMotor = FALSE;
static int currentMotor = MOTORFLAG_UNKOWN;

static unsigned int currentPulseNum = 0;

// 用于测试电机
static BOOL testMode = FALSE;
static BOOL controlTogether = FALSE;
static unsigned int testPulseSettingNum = 10000;

static BOOL userControl = FALSE;
static BOOL waitUserControl = FALSE;

// 如果在第一个电机结束，第二个电机运动之前断电，则保存当前状态
static BOOL saveFirstAction = FALSE;

static BOOL saveCurrentPosition = TRUE;

static int currentPage = 0;

void SetCurrentPage(int page)
{
	currentPage = page;
}

void SaveSnapshot()
{
	if (saveFirstAction)
	{
		saveFirstAction = FALSE;
		IapProgramByte(IAP_ADDRESS+100, 0xEE); //写入标志位
		IapProgramByte(IAP_ADDRESS+101, userControl);
		IapProgramByte(IAP_ADDRESS+102, waitUserControl);

		IapProgramByte(IAP_ADDRESS+103, (BYTE)(currentMotor>>8));
		IapProgramByte(IAP_ADDRESS+104, (BYTE)currentMotor);

		IapProgramByte(IAP_ADDRESS+105, (BYTE)(motorAction>>8));
		IapProgramByte(IAP_ADDRESS+106, (BYTE)motorAction);

		IapProgramByte(IAP_ADDRESS+107, (BYTE)(pulseSettingNum>>8));
		IapProgramByte(IAP_ADDRESS+108, (BYTE)pulseSettingNum);

		secondMotorFlag = 1;
	}
}

void RestoreSnapshot()
{
	if (IapReadByte(IAP_ADDRESS+100) == 0xEE)
	{	
		userControl = IapReadByte(IAP_ADDRESS+101);
		waitUserControl = IapReadByte(IAP_ADDRESS+102);
		currentMotor = ((IapReadByte(IAP_ADDRESS+103) << 8) | IapReadByte(IAP_ADDRESS+104));
		motorAction = ((IapReadByte(IAP_ADDRESS+105) << 8) | IapReadByte(IAP_ADDRESS+106));
		pulseSettingNum = ((IapReadByte(IAP_ADDRESS+107) << 8) | IapReadByte(IAP_ADDRESS+108));

		if (userControl == TRUE)
		{
			startSecondMotor = TRUE;	
		}
	}
}

void UserControl()
{
	if (motorAction != MOTORDIRECTION_FORWARD && motorAction != MOTORDIRECTION_BACKWARD)
	{
		return;
	}
	userControl = TRUE;
	StartSecondMotor();
}

static void DebugParameters()
{
	int i;
	DebugParameter("pulseSettingNum", pulseSettingNum);
	DebugParameter("pulseSettingFreq", pulseSettingFreq);
	DebugParameter("motorStepAngle", motorStepAngle);
	DebugParameter("motorReducGearRatio", motorReducGearRatio);
	DebugParameter("motorRotationAngle", motorRotationAngle);
	DebugParameter("screwPitch", screwPitch);
	DebugParameter("ballScrew", ballScrew);
	DebugParameter("currentPosition", currentPosition);
	DebugParameter("isStartPosition", IsInitPosition()); 
	for (i = 0; i < ROTATIONANGLE_SIZE; i++)
	{
		DebugParameter1(i + 1, rotationAngles[i]);
	} 
}

// 控制单片机引脚
void SetMotorForward()
{
	if (currentMotor == MOTORFLAG_TWO)
	{
		#ifndef REVERSE_DIRECTION
			secondMotorDirection = 1;
		#else
			secondMotorDirection = 0;
		#endif
	}
	else if (currentMotor == MOTORFLAG_ONE)
	{
		#ifndef REVERSE_DIRECTION
			motorDirection = 1;
		#else
			motorDirection = 0;
		#endif
	}
}

void SetMotorBackward()	
{
	if (currentMotor == MOTORFLAG_TWO)
	{
		#ifndef REVERSE_DIRECTION
			secondMotorDirection = 0;
		#else
			secondMotorDirection = 1;
		#endif
	}
	else if (currentMotor == MOTORFLAG_ONE)
	{
		#ifndef REVERSE_DIRECTION
			motorDirection = 0;
		#else
			motorDirection = 1;
		#endif
	}	
}

void DisablePulse()
{
	if (testMode == TRUE && controlTogether == TRUE)
	{
		secondMotorPWM = 1;
		motorPWM = 1;
	}
	else
	{
		if (currentMotor == MOTORFLAG_TWO)
		{
			secondMotorPWM = 1;
		}
		else if (currentMotor == MOTORFLAG_ONE)
		{
			motorPWM = 1;
		}
	}
}

void ChangePulse()
{
	if (testMode == TRUE && controlTogether == TRUE)
	{
		secondMotorPWM = ~secondMotorPWM;
		motorPWM = ~motorPWM;
	}
	else
	{
		if (currentMotor == MOTORFLAG_TWO)
		{
			secondMotorPWM = ~secondMotorPWM;
		}
		else if (currentMotor == MOTORFLAG_ONE)
		{
			motorPWM = ~motorPWM;
		}
	}
}

void UpdateParameters()
{
	unsigned long temp1 = 0;
	unsigned long temp2 = 0;
	
	switch (motorAction)
	{
	case MOTORDIRECTION_FORWARD:
		currentPosition++;
		// TODO:设置参数后可以计算并保存下来
		//计算电机旋转角:脉冲个数*电机步进角(pulseSettingNum * 1.8*100)
		temp1 = (unsigned long)currentPulseNum * 180;// 触摸屏通过串口都是以整数来发送�
		// 为什么使用pulseSettingNum不行?
		temp2 = (unsigned long)temp1 * screwPitch / 36000;
		motorRotationAngle  = temp1;
		ballScrew = temp2;
		break;
	case MOTORDIRECTION_BACKWARD:
		currentPosition--;
		// TODO:设置参数后可以计算并保存下来
		//计算电机旋转角:脉冲个数*电机步进角(pulseSettingNum * 1.8*100)
		temp1 = (unsigned long)currentPulseNum * 180;// 触摸屏通过串口都是以整数来发送�
		// 为什么使用pulseSettingNum不行?
		temp2 = (unsigned long)temp1 * screwPitch / 36000;
		motorRotationAngle = temp1;
		ballScrew = temp2;
		break;
	}	
}

void StartSecondMotor()
{
	if (/*startSecondMotor &&*/ userControl)
	{
		startSecondMotor = FALSE;
		userControl = FALSE;
		waitUserControl = FALSE;
		
		DebugParameter("StartSecondMotor motorAction", motorAction);
		// 等待电机停止
		//delay_ms(1000);
		if (motorAction == MOTORDIRECTION_FORWARD)
		{
			currentMotor = MOTORFLAG_TWO;
			SetMotorForward();
		}
		else if (motorAction == MOTORDIRECTION_BACKWARD)
		{
			currentMotor = MOTORFLAG_TWO;
			SetMotorBackward();
		}
		
		SyncRotationAngle();
		if (pulseSettingNum == 0)
		{
			return;
		}
		SetTimerParameter(50, pulseSettingNum);
		userControlFlag = 0;
	}
}

// 由Timer模块调用
void FinishPulse()
{
	// 等待用户输入，第二个电机才能运行
	if (waitUserControl)
	{
		return;
	}
	
	switch (motorAction)
	{
		case MOTOR_UNKOWN:
			return;
		case MOTOR_INIT:
			if (currentMotor == MOTORFLAG_ONE)
			{
				Debug("FinishPulse MOTORFLAG_ONE\r\n");
				motorAction = MOTOR_INIT;
				currentMotor = MOTORFLAG_TWO;
				/*
				#ifndef REVERSE_DIRECTION
					secondMotorDirection = 0; // 第二个电机后退
				#else
					secondMotorDirection = 1;
				#endif
				*/
				SetMotorBackward();
				SetTimerParameter(50, 65535);		
				waitUserControl = FALSE;
			}
			else if (currentMotor == MOTORFLAG_TWO)
			{
				Debug("FinishPulse MOTORFLAG_TWO\r\n");
				// 更新显示屏
				currentPosition = 1;
				currentMotor = MOTORFLAG_UNKOWN;
				motorAction = MOTOR_UNKOWN;
				// 不能立即刷新，否则会阻塞
				EnableRefreshDisplay();
				EnableSaveSetting();
				saveCurrentPosition = TRUE;
			}
			break;
		case MOTORDIRECTION_FORWARD:
		case MOTORDIRECTION_BACKWARD:
			if (currentMotor == MOTORFLAG_ONE)
			{
				// 第二个电机前进的同时第一个电机后退
				if (testMode == TRUE)
				{	
					#ifndef REVERSE_DIRECTION
						motorDirection = 0;
						secondMotorDirection = 1;
					#else
						motorDirection = 1;
						secondMotorDirection = 0;
					#endif
					currentMotor = MOTORFLAG_TWO;
					motorAction = MOTORDIRECTION_FORWARD;
					controlTogether = TRUE;
					SetTimerParameter(50, testPulseSettingNum);
					return;
				}
				waitUserControl = TRUE;
				startSecondMotor = TRUE;
				currentMotor = MOTORFLAG_TWO;

				saveFirstAction = TRUE;
				saveCurrentPosition = FALSE;
				EnableSaveSetting();
				
				//SaveParameters();
				
				secondMotorFlag = 1;
			}
			else if (currentMotor == MOTORFLAG_TWO)
			{
				//UpdateParameters();
				if (testMode == TRUE)
				{
					if (motorAction == MOTORDIRECTION_FORWARD)
					{
						#ifndef REVERSE_DIRECTION
							secondMotorDirection = 0;
						#else
							secondMotorDirection = 1;
						#endif
						
						controlTogether = FALSE;
						currentMotor = MOTORFLAG_TWO;
						motorAction = MOTORDIRECTION_BACKWARD;
						// 控制测试指示灯亮
						//secondMotorFlag = 0;
						SetTimerParameter(50, testPulseSettingNum);
					}
					else if (motorAction == MOTORDIRECTION_BACKWARD)
					{
						testMode = FALSE;
						isStartPosition1 = 1;
						isStartPosition2 = 1;
						currentMotor = MOTORFLAG_UNKOWN;
						motorAction = MOTOR_UNKOWN;
						// 控制测试指示灯灭
						//secondMotorFlag = 1;
						// 不能立即刷新，否则会阻塞
						EnableRefreshDisplay();
						//EnableSaveSetting();
					}
					return;
				}
				
				if (motorAction == MOTORDIRECTION_FORWARD)
					currentPosition++;
				else if (motorAction == MOTORDIRECTION_BACKWARD)
					currentPosition--;
				startSecondMotor = FALSE;
				currentMotor = MOTORFLAG_UNKOWN;
				motorAction = MOTOR_UNKOWN;
				// 不能立即刷新，否则会阻塞
				EnableRefreshDisplay();
				EnableSaveSetting(); // ---保存耗时会不会影响稳定性?

				userControlFlag = 1;
			}
			break;
		default:
			return;
	}	
}

//====================================
void InitHardware()
{
	uart_init();
	timer_init();
	InitDebug();
}

void EnableRefreshDisplay()
{
	refreshDisplay = TRUE;
}

void EnableSaveSetting()
{
	saveSetting = TRUE;
}

void DisableRefreshDisplay()
{
	refreshDisplay = FALSE;
}

void DisableSaveSetting()
{
	saveSetting = FALSE;
}

BOOL IsRefreshDisplay()
{
	return refreshDisplay;
}

BOOL IsSaveSetting()
{
	return saveSetting;
}

BOOL IsInitPosition()
{
	if (currentPosition == MOTORPOSITION_BEGIN)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

// 解析串口发送过来的命令，去除帧头和数据长度，只包含指令和数据两部分
// [5A A5 06] 83 00 00 01 01 F4 
void ParseCommand(unsigned char* buf, int len)
{
	int addr = 0;
	int size = 0;
	int dat = 0;
	// TODO: 参数检查
	len = len;
	// 变量地址，占用两个字节
	addr = ((buf[1] << 8) | buf[2]);
	// 变量的长度，占用1个字节
	size = buf[3];
	if (size != 0x01) return;
	// 变量的值，占用size个字
	dat = ((buf[4] << 8) | buf[5]);

	// 参数设置页面中输入框
	if (addr == 0x0000 || addr == 0x0002 || addr == 0x0004 || addr == 0x0006 || 
        addr == 0x0008 || addr == 0x000A || addr == 0x000C) 
	{
		DebugFunction("SetParameter", addr, dat);
		SetParameter(addr, dat);
	}
	// 旋转角设置页面中输入框
	else if (addr >= SCREEN_ROTATIONANGLE_BEGIN && addr <= SCREEN_ROTATIONANGLE_END)
	{
		DebugFunction("SetRotationAngle", addr, dat);
		SetRotationAngle(addr, dat);
	}
	// 返回按钮
	else if (addr == 0x000E || addr == 0x001E || addr == 0x002E)
	{
		DebugFunction("HandleReturn", addr, dat);
		HandleReturn(addr, dat);
	}
	// 电机控制按钮
	else if (addr == 0x0014 || addr == 0x0016 || addr == 0x0018)
	{
		DebugFunction("HandleMotor", addr, dat);
		HandleMotor(addr, dat);
	}
	// 其他按钮
	else if (addr == 0x00F0 || addr == 0x00F2 || addr == 0x00F4 || addr == 0x00F6) 
	{
		DebugFunction("HandleButtuon", addr, dat);
		HandleButtuon(addr, dat);
	}
	else
	{
		DebugFunction("error", addr, dat);
	}
}

void HandleButtuon(int addr, int dat)
{
	dat = dat;
	// 参数设置按钮
	if (addr == 0x00F0) 
	{
		ChangeScreenPage(0x02);
	}
	// 参数设置界面中更多按钮
	else if (addr == 0x00F2) 
	{ 
		ChangeScreenPage(0x07);
	}
	// 旋转角设置界面中下一页按钮
	else if (addr == 0x00F4) 
	{ 
		ChangeScreenPage(0x08);
	}
	// 旋转角设置界面中上一页按钮
	else if (addr == 0x00F6) 
	{ 
		ChangeScreenPage(0x07);
	}
	EnableRefreshDisplay();
}

void HandleReturn(int addr, int dat)
{
	int page = 0;
	dat = dat;
	// 切换屏幕
	if (addr == 0x000E)
	{
		page = 0x00;
	}
	else if (addr == 0x001E || addr == 0x002E)
	{ 
		page = 0x02;
		SyncRotationAngleNumber();
	} 
	
	if (inputChanged)
	{
		inputChanged = FALSE;
		ChangeScreenPage(0x04);
		// 为了显示保存界面，这里延时1s
		delay_ms(1000);
		SaveParametersToEEPROM();
	}
	ChangeScreenPage(page);
	EnableRefreshDisplay();// added 
}

void HandleMotor(int addr, int dat)
{
	if (addr == 0x0014) //初始化按钮
	{
		DebugFunction("InitMotor", addr, dat);
		InitMotor();
	}
	else if (addr == 0x0016) //后退按钮
	{
		DebugFunction("ControlMotor:backward", addr, dat);
		ControlMotor(MOTORDIRECTION_BACKWARD);
	}
	else if (addr == 0x0018) //前进按钮
	{
		DebugFunction("ControlMotor:forward", addr, dat);
		ControlMotor(MOTORDIRECTION_FORWARD);
	}
	else
	{
		dat = dat;
	}
}

void InitMotor()
{
	// 如果电机正在运行，不响应用户操作，也不记录用户操作
	if (motorAction != MOTOR_UNKOWN || startSecondMotor)
	{
		Debug("motor running\r\n");
		DebugParameter("motorAction", motorAction);
		return;
	}
	// 如果当前位置为1，则电机不需要运转
	if (currentPosition == MOTORPOSITION_BEGIN)
	{
		Debug("motor at begin\r\n");
		return;
	}

	Debug("InitMotor\r\n");
	motorAction = MOTOR_INIT;
	currentMotor = MOTORFLAG_ONE;
	SetMotorBackward();
	SetTimerParameter(50, 65535);	
	waitUserControl = FALSE;
	userControl = FALSE;
}

// TODO:当前位置范围为1-20，无法继续前进或后退时通过界面给出提示
void ControlMotor(int direction)
{
	int position = 0;
	#if 1
	// 当前电机未运行结束
	if (motorAction != MOTOR_UNKOWN/* || startSecondMotor*/)
	{
		Debug("motor running\r\n");
		DebugParameter("motorAction", motorAction);
		// 是否要记录操作
		return;
	}
	#endif
	
	DebugParameter("ControlMotor:direction", direction);

	userControl = FALSE;
	if (direction == MOTORDIRECTION_FORWARD)
	{
		position = currentPosition + 1;
		if (position > MOTORPOSITION_END || position <= MOTORPOSITION_BEGIN)
		{
			#if 0
				ChangeScreenPage(0x09);
				// 为了显示保存界面，这里延时1s
				delay_ms(1000);
				ChangeScreenPage(0x00);
			#endif
			return;
		}
		motorAction = MOTORDIRECTION_FORWARD;
		currentMotor = MOTORFLAG_ONE;
		SetMotorForward();
	}
	else if (direction == MOTORDIRECTION_BACKWARD)
	{
		position = currentPosition - 1;
		if (position < MOTORPOSITION_BEGIN ||  position >= MOTORPOSITION_END )
		{
			#if 0
			ChangeScreenPage(0x0A);
			// 为了显示保存界面，这里延时1s
			delay_ms(1000);
			ChangeScreenPage(0x00);
			#endif
			return;
		}
		motorAction = MOTORDIRECTION_BACKWARD;
		currentMotor = MOTORFLAG_ONE;
		SetMotorBackward();	
	}
	else
	{
		return;
	}
	SyncRotationAngle();
	if (pulseSettingNum == 0)
	{
		return;
	}
	SetTimerParameter(50, pulseSettingNum);

	secondMotorFlag = 0;
}

// 初始位置时才可以进行测试，测试的步进角固定
void TestMotor()
{
	#ifdef VERBOSE
		DebugParameter("testMode", testMode);
		DebugParameter("controlTogether", controlTogether);
		DebugParameter("isStartPosition1", isStartPosition1);
		DebugParameter("isStartPosition2", isStartPosition2);
		DebugParameter("currentPosition", currentPosition);
	#endif
	// isStartPosition1 == 1 && isStartPosition2 == 1?
	if (testMode != TRUE && IsInitPosition())
	{
	#ifdef VERBOSE
		Debug("TestMotor...\r\n");	
		Debug("TestMotor MOTORFLAG_ONE begin.\r\n");
	#endif
		testMode = TRUE;
		currentMotor = MOTORFLAG_ONE;
		motorAction = MOTORDIRECTION_FORWARD;

		#ifndef REVERSE_DIRECTION
			motorDirection = 1;
		#else
			motorDirection = 0;
		#endif
		controlTogether = FALSE;
		SetTimerParameter(50, testPulseSettingNum);
	}
}

// TODO:改写该函数
void GetSensorStatus()
{
	//初始位置传感器
	if (sensorStartPosi1 == 0)
	{
		if(isStartPosition1 == 0)
		{
			refreshDisplay = 1;
		}
		isStartPosition1 = 1;
	}
	if (sensorStartPosi1 == 1)
	{
		if(isStartPosition1 == 1)
		{
			refreshDisplay = 1;
		}
		isStartPosition1 = 0;
	}
	//初始位置传感器2
	if (sensorStartPosi2 == 0)
	{
		if(isStartPosition2 == 0)
		{
			refreshDisplay = 1;
		}
		isStartPosition2 = 1;
	}
	if (sensorStartPosi2 == 1)
	{
		if(isStartPosition2 == 1)
		{
			refreshDisplay = 1;
		}
		isStartPosition2 = 0;
	}
	//电磁铁输入
	if (electromagnetIn == 0)
	{
		electromagnetOut = 0;
	}
	else
	{
		electromagnetOut = 1;
	}
}

void CheckSensorInput()
{
#if 0
	//Get the sensor status
	GetSensorStatus();
	// ----如果两个传感器有一个有信号就说明是开始位置
	if (isStartPosition2 || isStartPosition1)
	{
		isStartPosition = 1;
	}
	/*
	else if (!isStartPosition2 && !isStartPosition1)
	{
		isStartPosition = 0;
	}
	*/
	//初始化过程中判断原点
	if (motorAction == MOTOR_INIT && (isStartPosition2 || isStartPosition1))
	{
		Debug("CheckSensorInput\r\n");
		isStartPosition1 = 0;
		isStartPosition2 = 0;
		initFlag = 0;
		//pulseSettingNumCount = 4;
		if (isStartPosition1)
		{

		}
		else if (isStartPosition2)
		{

		}
		//SetTimerParameter(0, 4);
		//SetTimerParameter(50, 4);
		StopPulseTimer();
	}
#endif

	if (motorAction == MOTOR_INIT)
	{
		if (currentMotor == MOTORFLAG_ONE)
		{
			if (sensorStartPosi1 == 0)
			{
				StopPulseTimer();
			}
		}
		else if (currentMotor == MOTORFLAG_TWO)
		{
			if (sensorStartPosi2 == 0)
			{
				StopPulseTimer();
				// 输出低电平给予用户反馈
				sensorStartFlag = 0;
				delay_ms(1);
				sensorStartFlag = 1;
			}
		}
	}
}

void ResetParameters()
{
	Debug("Reset parameters...\r\n");	
	pulseSettingNum = 0;
	pulseSettingFreq = 0;
	motorStepAngle = 0;
	motorRotationAngle = 0;
	motorReducGearRatio = 0;
	ballScrew = 0;
	screwPitch = 0;	
	currentPosition = 0;

	saveSetting = FALSE;
	refreshDisplay = FALSE;
	initFlag = 0;
}

void InitParameters()
{
	int i = 0;
	BOOL result = FALSE;
	ResetParameters();
	DebugParameters(); 	
	result = ReadParametersFromEEPROM();
	if (!result) // 使用默认值
	{
		Debug("Use default parameters\r\n");
		pulseSettingNum = 400; //脉冲个数
		pulseSettingFreq = 400; //脉冲频率
		motorStepAngle = 180; //电机步进角
		motorRotationAngle = 36000; //电机旋转角
		motorReducGearRatio = 100; //电机减速比
		screwPitch = 200; //丝杆丝距
		ballScrew = 200; //丝杆导程	
		currentPosition = MOTORPOSITION_BEGIN; //当前位置 1~20

		for (i = 0; i < ROTATIONANGLE_SIZE; i++)
		{
			rotationAngles[i] = 0x00;//i * 100;
		} 	
		SaveParametersToEEPROM();
	} 

	SyncRotationAngleNumber();
	EnableRefreshDisplay();
	DebugParameters();
}

static unsigned int GetPulseNumber(unsigned int position)
{
	//DebugParameter("currentPosition", currentPosition);
	return rotationAngles[position - 1];
}

static unsigned long GetRotationAngle(unsigned int position)
{
	unsigned int currentPulseNumber = 0;
	unsigned long currentRotationAngle = 0;
	unsigned long realMotorStepAngle = 0;
	currentPulseNumber = rotationAngles[position - 1];
	/*
		为了避免浮点数计算，扩大1000倍
		实际表达式:(motorStepAngle/1000)/(motorReducGearRatio/100)*1000
		简化表达式:100*motorStepAngle/motorReducGearRatio
	*/ 
	realMotorStepAngle = (unsigned long)100 * motorStepAngle / motorReducGearRatio;
	/*
		实际表达式:currentPulseNumber*realMotorStepAngle/1000*100
		简化表达式:currentPulseNumber*realMotorStepAngle/10
	*/
	currentRotationAngle = (unsigned long)currentPulseNumber * realMotorStepAngle / 10;
	return currentRotationAngle;
}

static unsigned long GetBallScrew(unsigned int position)
{
	unsigned int i = 0;
	unsigned long currentBallScrew = 0;
	unsigned int totalPulseNumber = 0;
	unsigned int pulseNumber = 0;
	unsigned long realMotorStepAngle = 0;
	if (position == 1)
	{
		return 0;
	}
	for (i = 1; i < position; i++)
	{
		pulseNumber = GetPulseNumber(i);
		totalPulseNumber += pulseNumber;
		#if 0
		DebugParameter("i", i);
		DebugParameter("pulseNumber", pulseNumber);
		#endif
	}
	/*
		为了避免浮点数计算，扩大1000倍
		实际表达式:(motorStepAngle/1000)/(motorReducGearRatio/100)*1000
		简化表达式:100*motorStepAngle/motorReducGearRatio
	*/ 
	realMotorStepAngle = (unsigned long)100 * motorStepAngle / motorReducGearRatio;
	/*
		实际表达式:totalPulseNumber*realMotorStepAngle*(screwPitch/100)/360/1000*100
		简化表达式:totalPulseNumber*realMotorStepAngle*screwPitch/360000
	*/
	currentBallScrew = (unsigned long)totalPulseNumber * realMotorStepAngle * screwPitch / 360000;
	return currentBallScrew;
}


// 更新显示数据
static void GetRefreshParameters()
{
	pulseSettingNum = GetPulseNumber(currentPosition);
	motorRotationAngle = GetRotationAngle(currentPosition);
	ballScrew = GetBallScrew(currentPosition);
}

// 刷新指定屏幕的数据
static void RefreshParametersByPage(int page)
{
	int i;
	int addr;
	switch (page)
	{
		case 0x00:
			GetRefreshParameters();
			// modified 0x0010->0x0020 0x0012->0x0022
			SendDataToScreen(0x0020, currentPosition);
			SendDataToScreen(0x0022, IsInitPosition()); 
			SendLongDataToScreen(0x000A, ballScrew);
			SendLongDataToScreen(0x000E, motorRotationAngle);
			break;
			
		case 0x02:
			GetRefreshParameters();
			SendDataToScreen(0x0000, pulseSettingNum);
			SendDataToScreen(0x0002, pulseSettingFreq);
			SendDataToScreen(0x0004, motorStepAngle);
			SendDataToScreen(0x0006, screwPitch);
			SendDataToScreen(0x0008, motorReducGearRatio);
			SendLongDataToScreen(0x000A, ballScrew);
			SendLongDataToScreen(0x000E, motorRotationAngle);
			break;
			
		case 0x07:
			for (i = 0; i < ROTATIONANGLE_SIZE/2; i++)
			{
				addr = SCREEN_ROTATIONANGLE_BEGIN + i * 2;
				SendDataToScreen(addr, rotationAngles[i]);
			} 
			break;
			
		case 0x08:
			for (i = ROTATIONANGLE_SIZE/2; i < ROTATIONANGLE_SIZE; i++)
			{
				addr = SCREEN_ROTATIONANGLE_BEGIN + i * 2;
				SendDataToScreen(addr, rotationAngles[i]);
			} 
			break;
	} 	
}

// 刷新所有数据
static void RefreshParameters()
{
	int i;
	int addr;
	DebugParameter("currentPosition", currentPosition);
	//pulseSettingNum = GetPulseNumber(currentPosition);
	//motorRotationAngle = GetRotationAngle(currentPosition);
	//ballScrew = GetBallScrew(currentPosition);
	GetRefreshParameters();
	SendDataToScreen(0x0000, pulseSettingNum);
	SendDataToScreen(0x0002, pulseSettingFreq);
	SendDataToScreen(0x0004, motorStepAngle);
	SendDataToScreen(0x0006, screwPitch);
	SendDataToScreen(0x0008, motorReducGearRatio);
	SendLongDataToScreen(0x000A, ballScrew);// ballScrew
	SendLongDataToScreen(0x000E, motorRotationAngle); // motorRotationAngle

	// modified 0x0010->0x0020 0x0012->0x0022
	SendDataToScreen(0x0020, currentPosition);
	SendDataToScreen(0x0022, IsInitPosition()); 
	for (i = 0; i < ROTATIONANGLE_SIZE; i++)
	{
		addr = SCREEN_ROTATIONANGLE_BEGIN + i * 2;
		SendDataToScreen(addr, rotationAngles[i]);
	}
}

void DisplayParameters()
{
	if (IsRefreshDisplay())
	{
		RefreshParametersByPage(currentPage);
		DisableRefreshDisplay();
	}
}

void SaveParameters()
{
	if (IsSaveSetting())
	{
		SaveParametersToEEPROM();
		SaveSnapshot();
		DisableSaveSetting();
	}
}

BOOL ReadParametersFromEEPROM()
{
	int i = 0;
	int addr = 0;
	BOOL result = FALSE;
	delay_ms(10); 
	Debug("Read parameters from EEPROM\r\n");
	// 如果地址200的内容不是0XEE,即没有写入标识，表示第一次读取
	if (IapReadByte(IAP_ADDRESS+200) == 0xEE)
	{	
		pulseSettingNum = ((IapReadByte(IAP_ADDRESS+0) << 8) | IapReadByte(IAP_ADDRESS+1));
		pulseSettingFreq = ((IapReadByte(IAP_ADDRESS+2) << 8) | IapReadByte(IAP_ADDRESS+3));
		motorStepAngle = ((IapReadByte(IAP_ADDRESS+4) << 8) | IapReadByte(IAP_ADDRESS+5));
		screwPitch = ((IapReadByte(IAP_ADDRESS+6) << 8) | IapReadByte(IAP_ADDRESS+7));
		motorReducGearRatio = ((IapReadByte(IAP_ADDRESS+8) << 8) | IapReadByte(IAP_ADDRESS+9));
		
		ballScrew = ((IapReadByte(IAP_ADDRESS+10) << 24) | (IapReadByte(IAP_ADDRESS+11) << 16) + 
					 (IapReadByte(IAP_ADDRESS+12) << 8) + IapReadByte(IAP_ADDRESS+13));
		motorRotationAngle = ((IapReadByte(IAP_ADDRESS+14) << 24) | (IapReadByte(IAP_ADDRESS+15) << 16) + 
					 (IapReadByte(IAP_ADDRESS+16) << 8) + IapReadByte(IAP_ADDRESS+17));
		// 读取当前位置
		currentPosition = IapReadByte(IAP_ADDRESS+18);
		// 读取旋转角
		addr = IAP_ADDRESS + 20;
		for (i = 0; i < ROTATIONANGLE_SIZE; i++)
		{
			rotationAngles[i] = ((IapReadByte(addr+0) << 8) | IapReadByte(addr+1));
			addr += 2;
		}
		
		result = TRUE;
	}

	RestoreSnapshot();
	return result;
}

// TODO:每次只保存修改的数据
BOOL SaveParametersToEEPROM()
{
	int i = 0;
	int addr = 0;
	BOOL result = FALSE;
    //delay_ms(10); 
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
	IapProgramByte(IAP_ADDRESS+10, (BYTE)((ballScrew>>24) & 0xFF));
	IapProgramByte(IAP_ADDRESS+11, (BYTE)((ballScrew>>16) & 0xFF));
	IapProgramByte(IAP_ADDRESS+12, (BYTE)((ballScrew>>8)  & 0xFF));
	IapProgramByte(IAP_ADDRESS+13, (BYTE)(ballScrew       & 0xFF));

	IapProgramByte(IAP_ADDRESS+14, (BYTE)((motorRotationAngle>>24) & 0xFF));
	IapProgramByte(IAP_ADDRESS+15, (BYTE)((motorRotationAngle>>16) & 0xFF));
	IapProgramByte(IAP_ADDRESS+16, (BYTE)((motorRotationAngle>>8)  & 0xFF));
	IapProgramByte(IAP_ADDRESS+17, (BYTE)(motorRotationAngle       & 0xFF));
	    
	// 新增
	IapProgramByte(IAP_ADDRESS+18, (BYTE)currentPosition);
	

	// 保存旋转角
	addr = IAP_ADDRESS + 20;
	for (i = 0; i < ROTATIONANGLE_SIZE; i++)
	{
		IapProgramByte(addr+0, (BYTE)(rotationAngles[i]>>8));
		IapProgramByte(addr+1, (BYTE)rotationAngles[i]);
		addr += 2;
	}

	IapProgramByte(IAP_ADDRESS+200, 0xEE); //写入标志位
	result = TRUE;
	return result;
}

void SetParameter(int key, int dat)
{
	if (key == 0x0000)//脉冲个数
	{
		if (dat != pulseSettingNum)
		{
			pulseSettingNum = dat;
			inputChanged = TRUE;
		}
	}
	else if (key == 0x0002) //脉冲频率
	{
		if (dat != pulseSettingFreq)
		{
			pulseSettingFreq = dat;
			inputChanged = TRUE;
		}
	}
	else if (key == 0x0004) //电机步进角
	{
		if (dat != motorStepAngle)
		{
			motorStepAngle = dat;
			inputChanged = TRUE;
		}
	}
	else if (key == 0x0006) // 丝杆丝距
	{
		if (dat != screwPitch)
		{
			screwPitch = dat;
			inputChanged = TRUE;
		}
	}
	else if (key == 0x0008) //电机减速比
	{
		if (dat != motorReducGearRatio)
		{
			motorReducGearRatio = dat;
			inputChanged = TRUE;
		}
	}
	else if (key == 0x000A) //丝杆导程
	{
		if (dat != ballScrew)
		{
			ballScrew = dat;
			inputChanged = TRUE;
		}
	}
	else if (key == 0x000C) //电机旋转角
	{
		if (dat != motorRotationAngle)
		{
			motorRotationAngle = dat;
			inputChanged = TRUE;
		}
	}
}

void SetRotationAngle(int key, int dat)
{
	int index = (key & 0x00FF) / 2;
	DebugParameter("index", index);
	if (dat != rotationAngles[index])
	{
		rotationAngles[index] = dat;
		inputChanged = TRUE;
	}
}

// 查找数组中最后一个不为0的元素，记录其下标
void SyncRotationAngleNumber()
{
	int i = ROTATIONANGLE_SIZE -1;
	while ((i >= 0) && (rotationAngles[i] == 0))
	{
		i--;
	}
	rotationAngleNumber = i + 1;
	DebugParameter("rotationAngleNumber", rotationAngleNumber);
	//DebugParameter("rotationAngle", rotationAngles[rotationAngleNumber - 1]);
	// 
	testPulseSettingNum = rotationAngles[0];
}

// 通过设置脉冲数来调整旋转角
void SyncRotationAngle()
{
	unsigned int rotationAngle;
	#if 0
		unsigned int i;
		// 策略1:循环使用旋转角
		if (rotationAngleNumber > 0)
		{
			for (i = 0; i < rotationAngleNumber; i++)
			{
				rotationAngle = rotationAngles[currentRotationAngleIndex];
				currentRotationAngleIndex = (currentRotationAngleIndex + 1) % rotationAngleNumber;
				if (rotationAngle > 0)
				{
					pulseSettingNum = rotationAngle;
					return;
				}
			}
		}
	#else 
		// 策略2:每个位置对应一个旋转角
		// 位置1对应旋转角1，...
		if (motorAction == MOTORDIRECTION_FORWARD)
		{
			rotationAngle = rotationAngles[currentPosition - 1];
		}
		else if (motorAction == MOTORDIRECTION_BACKWARD)
		{
			rotationAngle = rotationAngles[currentPosition - 1 - 1];
		}		
		if (rotationAngle == 65535)
			rotationAngle = 0;
		pulseSettingNum = rotationAngle;
		currentPulseNum = rotationAngle;
	#endif
}


