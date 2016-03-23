#include "parameter.h"
#include "screen.h"
#include "eeprom.h"
#include "timer.h"
#include "uart.h"
#include "debug.h"

#define REVERSE_DIRECTION

// µÚ¶þµç»úÔËÐÐÊÇ·ñÐèÒªÓÉÓÃ»§¿ØÖÆ
//#define USER_CONTROL 

//TODO:Ê¹ÓÃ½á¹¹Ìå·â×°
/***************************************************************************/
// ²ÎÊý¶¨Òå	
/***************************************************************************/
unsigned char currentPosition;  //µ±Ç°Î»ÖÃ 1~20
unsigned int  pulseSettingNum;  //Âö³å¸öÊý
unsigned int  pulseSettingFreq; //Âö³åÆµÂÊ
unsigned int  motorStepAngle;   //µç»ú²½½ø½Ç
unsigned int  screwPitch;       //Ë¿¸ËË¿¾à
unsigned int  motorReducGearRatio; //µç»ú¼õËÙ±È

unsigned long  ballScrew;          //Ë¿¸Ëµ¼³Ì
unsigned long  motorRotationAngle; //µç»úÐý×ª½Ç
unsigned char isStartPosition;     //³õÊ¼Î»ÖÃ
unsigned char isStartPosition1;    //³õÊ¼Î»ÖÃ
unsigned char isStartPosition2;    //³õÊ¼Î»ÖÃ

// unused
unsigned char initFlag = 0; //³õÊ¼»¯±êÖ¾Î»

BOOL refreshDisplay = FALSE; //Ë¢ÐÂÆÁÄ»±êÖ¾Î» 0 ²»Ë¢ÐÂ 1Ë¢ÐÂ
BOOL saveSetting = FALSE; // ±£´æ²ÎÊý±êÖ¾Î»

static BOOL inputChanged = FALSE;
static int  motorAction = MOTOR_UNKOWN;
static unsigned int rotationAngles[ROTATIONANGLE_SIZE] = {0};
static int rotationAngleNumber = 0;
static int currentRotationAngleIndex = -1;

// ÓÃÓÚÖ§³ÖÁíÍâÒ»¸öµç»ú
static BOOL startSecondMotor = FALSE;
static int currentMotor = MOTORFLAG_UNKOWN;

static unsigned int currentPulseNum = 0;

// ÓÃÓÚ²âÊÔµç»ú
static BOOL testMode = FALSE;
static BOOL controlTogether = FALSE;
static unsigned int testPulseSettingNum = 10000;

static BOOL userControl = FALSE;
static BOOL waitUserControl = FALSE;

// Èç¹ûÔÚµÚÒ»¸öµç»ú½áÊø£¬µÚ¶þ¸öµç»úÔË¶¯Ö®Ç°¶Ïµç£¬Ôò±£´æµ±Ç°×´Ì¬
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
		IapProgramByte(IAP_ADDRESS+100, 0xEE); //Ð´Èë±êÖ¾Î»
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

// ¿ØÖÆµ¥Æ¬»úÒý½Å
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
		// TODO:ÉèÖÃ²ÎÊýºó¿ÉÒÔ¼ÆËã²¢±£´æÏÂÀ´
		//¼ÆËãµç»úÐý×ª½Ç:Âö³å¸öÊý*µç»ú²½½ø½Ç(pulseSettingNum * 1.8*100)
		temp1 = (unsigned long)currentPulseNum * 180;// ´¥ÃþÆÁÍ¨¹ý´®¿Ú¶¼ÊÇÒÔÕûÊýÀ´·¢ËÍ£
		// ÎªÊ²Ã´Ê¹ÓÃpulseSettingNum²»ÐÐ?
		temp2 = (unsigned long)temp1 * screwPitch / 36000;
		motorRotationAngle  = temp1;
		ballScrew = temp2;
		break;
	case MOTORDIRECTION_BACKWARD:
		currentPosition--;
		// TODO:ÉèÖÃ²ÎÊýºó¿ÉÒÔ¼ÆËã²¢±£´æÏÂÀ´
		//¼ÆËãµç»úÐý×ª½Ç:Âö³å¸öÊý*µç»ú²½½ø½Ç(pulseSettingNum * 1.8*100)
		temp1 = (unsigned long)currentPulseNum * 180;// ´¥ÃþÆÁÍ¨¹ý´®¿Ú¶¼ÊÇÒÔÕûÊýÀ´·¢ËÍ£
		// ÎªÊ²Ã´Ê¹ÓÃpulseSettingNum²»ÐÐ?
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
		// µÈ´ýµç»úÍ£Ö¹
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

// ÓÉTimerÄ£¿éµ÷ÓÃ
void FinishPulse()
{
	// µÈ´ýÓÃ»§ÊäÈë£¬µÚ¶þ¸öµç»ú²ÅÄÜÔËÐÐ
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
					secondMotorDirection = 0; // µÚ¶þ¸öµç»úºóÍË
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
				// ¸üÐÂÏÔÊ¾ÆÁ
				currentPosition = 1;
				currentMotor = MOTORFLAG_UNKOWN;
				motorAction = MOTOR_UNKOWN;
				// ²»ÄÜÁ¢¼´Ë¢ÐÂ£¬·ñÔò»á×èÈû
				EnableRefreshDisplay();
				EnableSaveSetting();
				saveCurrentPosition = TRUE;
			}
			break;
		case MOTORDIRECTION_FORWARD:
		case MOTORDIRECTION_BACKWARD:
			if (currentMotor == MOTORFLAG_ONE)
			{
				// µÚ¶þ¸öµç»úÇ°½øµÄÍ¬Ê±µÚÒ»¸öµç»úºóÍË
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
						// ¿ØÖÆ²âÊÔÖ¸Ê¾µÆÁÁ
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
						// ¿ØÖÆ²âÊÔÖ¸Ê¾µÆÃð
						//secondMotorFlag = 1;
						// ²»ÄÜÁ¢¼´Ë¢ÐÂ£¬·ñÔò»á×èÈû
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
				// ²»ÄÜÁ¢¼´Ë¢ÐÂ£¬·ñÔò»á×èÈû
				EnableRefreshDisplay();
				EnableSaveSetting(); // ---±£´æºÄÊ±»á²»»áÓ°ÏìÎÈ¶¨ÐÔ?

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

// ½âÎö´®¿Ú·¢ËÍ¹ýÀ´µÄÃüÁî£¬È¥³ýÖ¡Í·ºÍÊý¾Ý³¤¶È£¬Ö»°üº¬Ö¸ÁîºÍÊý¾ÝÁ½²¿·Ö
// [5A A5 06] 83 00 00 01 01 F4 
void ParseCommand(unsigned char* buf, int len)
{
	int addr = 0;
	int size = 0;
	int dat = 0;
	// TODO: ²ÎÊý¼ì²é
	len = len;
	// ±äÁ¿µØÖ·£¬Õ¼ÓÃÁ½¸ö×Ö½Ú
	addr = ((buf[1] << 8) | buf[2]);
	// ±äÁ¿µÄ³¤¶È£¬Õ¼ÓÃ1¸ö×Ö½Ú
	size = buf[3];
	if (size != 0x01) return;
	// ±äÁ¿µÄÖµ£¬Õ¼ÓÃsize¸ö×Ö
	dat = ((buf[4] << 8) | buf[5]);

	// ²ÎÊýÉèÖÃÒ³ÃæÖÐÊäÈë¿ò
	if (addr == 0x0000 || addr == 0x0002 || addr == 0x0004 || addr == 0x0006 || 
        addr == 0x0008 || addr == 0x000A || addr == 0x000C) 
	{
		DebugFunction("SetParameter", addr, dat);
		SetParameter(addr, dat);
	}
	// Ðý×ª½ÇÉèÖÃÒ³ÃæÖÐÊäÈë¿ò
	else if (addr >= SCREEN_ROTATIONANGLE_BEGIN && addr <= SCREEN_ROTATIONANGLE_END)
	{
		DebugFunction("SetRotationAngle", addr, dat);
		SetRotationAngle(addr, dat);
	}
	// ·µ»Ø°´Å¥
	else if (addr == 0x000E || addr == 0x001E || addr == 0x002E)
	{
		DebugFunction("HandleReturn", addr, dat);
		HandleReturn(addr, dat);
	}
	// µç»ú¿ØÖÆ°´Å¥
	else if (addr == 0x0014 || addr == 0x0016 || addr == 0x0018)
	{
		DebugFunction("HandleMotor", addr, dat);
		HandleMotor(addr, dat);
	}
	// ÆäËû°´Å¥
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
	// ²ÎÊýÉèÖÃ°´Å¥
	if (addr == 0x00F0) 
	{
		ChangeScreenPage(0x02);
	}
	// ²ÎÊýÉèÖÃ½çÃæÖÐ¸ü¶à°´Å¥
	else if (addr == 0x00F2) 
	{ 
		ChangeScreenPage(0x07);
	}
	// Ðý×ª½ÇÉèÖÃ½çÃæÖÐÏÂÒ»Ò³°´Å¥
	else if (addr == 0x00F4) 
	{ 
		ChangeScreenPage(0x08);
	}
	// Ðý×ª½ÇÉèÖÃ½çÃæÖÐÉÏÒ»Ò³°´Å¥
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
	// ÇÐ»»ÆÁÄ»
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
		// ÎªÁËÏÔÊ¾±£´æ½çÃæ£¬ÕâÀïÑÓÊ±1s
		delay_ms(1000);
		SaveParametersToEEPROM();
	}
	ChangeScreenPage(page);
	EnableRefreshDisplay();// added 
}

void HandleMotor(int addr, int dat)
{
	if (addr == 0x0014) //³õÊ¼»¯°´Å¥
	{
		DebugFunction("InitMotor", addr, dat);
		InitMotor();
	}
	else if (addr == 0x0016) //ºóÍË°´Å¥
	{
		DebugFunction("ControlMotor:backward", addr, dat);
		ControlMotor(MOTORDIRECTION_BACKWARD);
	}
	else if (addr == 0x0018) //Ç°½ø°´Å¥
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
	// Èç¹ûµç»úÕýÔÚÔËÐÐ£¬²»ÏìÓ¦ÓÃ»§²Ù×÷£¬Ò²²»¼ÇÂ¼ÓÃ»§²Ù×÷
	if (motorAction != MOTOR_UNKOWN || startSecondMotor)
	{
		Debug("motor running\r\n");
		DebugParameter("motorAction", motorAction);
		return;
	}
	// Èç¹ûµ±Ç°Î»ÖÃÎª1£¬Ôòµç»ú²»ÐèÒªÔË×ª
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

// TODO:µ±Ç°Î»ÖÃ·¶Î§Îª1-20£¬ÎÞ·¨¼ÌÐøÇ°½ø»òºóÍËÊ±Í¨¹ý½çÃæ¸ø³öÌáÊ¾
void ControlMotor(int direction)
{
	int position = 0;
	#if 1
	// µ±Ç°µç»úÎ´ÔËÐÐ½áÊø
	if (motorAction != MOTOR_UNKOWN/* || startSecondMotor*/)
	{
		Debug("motor running\r\n");
		DebugParameter("motorAction", motorAction);
		// ÊÇ·ñÒª¼ÇÂ¼²Ù×÷
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
				// ÎªÁËÏÔÊ¾±£´æ½çÃæ£¬ÕâÀïÑÓÊ±1s
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
			// ÎªÁËÏÔÊ¾±£´æ½çÃæ£¬ÕâÀïÑÓÊ±1s
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

// ³õÊ¼Î»ÖÃÊ±²Å¿ÉÒÔ½øÐÐ²âÊÔ£¬²âÊÔµÄ²½½ø½Ç¹Ì¶¨
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

// TODO:¸ÄÐ´¸Ãº¯Êý
void GetSensorStatus()
{
	//³õÊ¼Î»ÖÃ´«¸ÐÆ÷
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
	//³õÊ¼Î»ÖÃ´«¸ÐÆ÷2
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
	//µç´ÅÌúÊäÈë
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
	// ----Èç¹ûÁ½¸ö´«¸ÐÆ÷ÓÐÒ»¸öÓÐÐÅºÅ¾ÍËµÃ÷ÊÇ¿ªÊ¼Î»ÖÃ
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
	//³õÊ¼»¯¹ý³ÌÖÐÅÐ¶ÏÔ­µã
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
				// Êä³öµÍµçÆ½¸øÓèÓÃ»§·´À¡
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
	if (!result) // Ê¹ÓÃÄ¬ÈÏÖµ
	{
		Debug("Use default parameters\r\n");
		pulseSettingNum = 400; //Âö³å¸öÊý
		pulseSettingFreq = 400; //Âö³åÆµÂÊ
		motorStepAngle = 180; //µç»ú²½½ø½Ç
		motorRotationAngle = 36000; //µç»úÐý×ª½Ç
		motorReducGearRatio = 100; //µç»ú¼õËÙ±È
		screwPitch = 200; //Ë¿¸ËË¿¾à
		ballScrew = 200; //Ë¿¸Ëµ¼³Ì	
		currentPosition = MOTORPOSITION_BEGIN; //µ±Ç°Î»ÖÃ 1~20

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
		ÎªÁË±ÜÃâ¸¡µãÊý¼ÆËã£¬À©´ó1000±¶
		Êµ¼Ê±í´ïÊ½:(motorStepAngle/1000)/(motorReducGearRatio/100)*1000
		¼ò»¯±í´ïÊ½:100*motorStepAngle/motorReducGearRatio
	*/ 
	realMotorStepAngle = (unsigned long)100 * motorStepAngle / motorReducGearRatio;
	/*
		Êµ¼Ê±í´ïÊ½:currentPulseNumber*realMotorStepAngle/1000*100
		¼ò»¯±í´ïÊ½:currentPulseNumber*realMotorStepAngle/10
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
		ÎªÁË±ÜÃâ¸¡µãÊý¼ÆËã£¬À©´ó1000±¶
		Êµ¼Ê±í´ïÊ½:(motorStepAngle/1000)/(motorReducGearRatio/100)*1000
		¼ò»¯±í´ïÊ½:100*motorStepAngle/motorReducGearRatio
	*/ 
	realMotorStepAngle = (unsigned long)100 * motorStepAngle / motorReducGearRatio;
	/*
		Êµ¼Ê±í´ïÊ½:totalPulseNumber*realMotorStepAngle*(screwPitch/100)/360/1000*100
		¼ò»¯±í´ïÊ½:totalPulseNumber*realMotorStepAngle*screwPitch/360000
	*/
	currentBallScrew = (unsigned long)totalPulseNumber * realMotorStepAngle * screwPitch / 360000;
	return currentBallScrew;
}


// ¸üÐÂÏÔÊ¾Êý¾Ý
static void GetRefreshParameters()
{
	pulseSettingNum = GetPulseNumber(currentPosition);
	motorRotationAngle = GetRotationAngle(currentPosition);
	ballScrew = GetBallScrew(currentPosition);
}

// Ë¢ÐÂÖ¸¶¨ÆÁÄ»µÄÊý¾Ý
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

// Ë¢ÐÂËùÓÐÊý¾Ý
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
	// Èç¹ûµØÖ·200µÄÄÚÈÝ²»ÊÇ0XEE,¼´Ã»ÓÐÐ´Èë±êÊ¶£¬±íÊ¾µÚÒ»´Î¶ÁÈ¡
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
		// ¶ÁÈ¡µ±Ç°Î»ÖÃ
		currentPosition = IapReadByte(IAP_ADDRESS+18);
		// ¶ÁÈ¡Ðý×ª½Ç
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

// TODO:Ã¿´ÎÖ»±£´æÐÞ¸ÄµÄÊý¾Ý
BOOL SaveParametersToEEPROM()
{
	int i = 0;
	int addr = 0;
	BOOL result = FALSE;
    //delay_ms(10); 
	IapEraseSector(IAP_ADDRESS); //²Á³ýEEPROM
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
	    
	// ÐÂÔö
	IapProgramByte(IAP_ADDRESS+18, (BYTE)currentPosition);
	

	// ±£´æÐý×ª½Ç
	addr = IAP_ADDRESS + 20;
	for (i = 0; i < ROTATIONANGLE_SIZE; i++)
	{
		IapProgramByte(addr+0, (BYTE)(rotationAngles[i]>>8));
		IapProgramByte(addr+1, (BYTE)rotationAngles[i]);
		addr += 2;
	}

	IapProgramByte(IAP_ADDRESS+200, 0xEE); //Ð´Èë±êÖ¾Î»
	result = TRUE;
	return result;
}

void SetParameter(int key, int dat)
{
	if (key == 0x0000)//Âö³å¸öÊý
	{
		if (dat != pulseSettingNum)
		{
			pulseSettingNum = dat;
			inputChanged = TRUE;
		}
	}
	else if (key == 0x0002) //Âö³åÆµÂÊ
	{
		if (dat != pulseSettingFreq)
		{
			pulseSettingFreq = dat;
			inputChanged = TRUE;
		}
	}
	else if (key == 0x0004) //µç»ú²½½ø½Ç
	{
		if (dat != motorStepAngle)
		{
			motorStepAngle = dat;
			inputChanged = TRUE;
		}
	}
	else if (key == 0x0006) // Ë¿¸ËË¿¾à
	{
		if (dat != screwPitch)
		{
			screwPitch = dat;
			inputChanged = TRUE;
		}
	}
	else if (key == 0x0008) //µç»ú¼õËÙ±È
	{
		if (dat != motorReducGearRatio)
		{
			motorReducGearRatio = dat;
			inputChanged = TRUE;
		}
	}
	else if (key == 0x000A) //Ë¿¸Ëµ¼³Ì
	{
		if (dat != ballScrew)
		{
			ballScrew = dat;
			inputChanged = TRUE;
		}
	}
	else if (key == 0x000C) //µç»úÐý×ª½Ç
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

// ²éÕÒÊý×éÖÐ×îºóÒ»¸ö²»Îª0µÄÔªËØ£¬¼ÇÂ¼ÆäÏÂ±ê
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

// Í¨¹ýÉèÖÃÂö³åÊýÀ´µ÷ÕûÐý×ª½Ç
void SyncRotationAngle()
{
	unsigned int rotationAngle;
	#if 0
		unsigned int i;
		// ²ßÂÔ1:Ñ­»·Ê¹ÓÃÐý×ª½Ç
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
		// ²ßÂÔ2:Ã¿¸öÎ»ÖÃ¶ÔÓ¦Ò»¸öÐý×ª½Ç
		// Î»ÖÃ1¶ÔÓ¦Ðý×ª½Ç1£¬...
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


