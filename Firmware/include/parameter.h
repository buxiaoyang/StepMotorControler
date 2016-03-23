/***************************************************************************/
// ������ϴ�豸������
// ģ�飺����ͷ�ļ�
// �ļ���parameter.h
// ���ߣ������D
// �汾��V0.02
// ���ڣ�2013��11��8��
// ���ܣ�����ͷ�ļ�
// оƬ��STC12C5A60S2
// ���룺Keil uVision3 V3.90
/***************************************************************************/

#ifndef __PARAMETER_H__
#define __PARAMETER_H__

#include "basefunc.h"

/***************************************************************************/
// ���Ŷ���
/***************************************************************************/
//����
sbit sensorStartPosi1 = P0^0; //��ʼλ�ô�����1
sbit sensorStartPosi2 = P0^1; //��ʼλ�ô�����2
sbit keyMotorForward  = P0^2; //�����ǰ�ź�
sbit keyMotorBackward = P0^3; //�������ź�
sbit electromagnetIn  = P0^4; //���������

// added by leitiannet
sbit keyTestMotor     = P0^5; //��������ź�
sbit keyInitMotor     = P0^6; //�����ʼ����Ϣ
sbit keyUserControl   = P0^7; //�ȴ��û����룬�ڶ�������ſ�ʼ��ת

sbit secondMotorFlag  = P1^7; //�ڶ��������ʼ��תָʾ��
sbit userControlFlag  = P1^6; //

sbit sensorStartFlag  = P1^4; //����͵�ƽ�����û�����

// end

//���
sbit motorDirection = P1^0;   //�������
sbit electromagnetOut = P1^1; //��������
sbit motorPWM = P2^6;         //�������

// TODO:����ڶ�������ӿ�
sbit secondMotorDirection = P2^0;   //�������
sbit secondMotorPWM = P2^1;         //�������

sbit testOut = P3^7;

#if 0
extern unsigned char currentPosition; //��ǰλ�� 1~20
extern unsigned int pulseSettingNum; //�������
extern unsigned int pulseSettingFreq; //����Ƶ��
extern unsigned int motorStepAngle; //���������
extern unsigned int screwPitch; //˿��˿��
extern unsigned int motorReducGearRatio; //������ٱ�
extern unsigned int ballScrew; //˿�˵���
extern unsigned int motorRotationAngle; //�����ת��
extern unsigned char isStartPosition; //��ʼλ��
extern unsigned char isStartPosition1; //��ʼλ��
extern unsigned char isStartPosition2; //��ʼλ��

extern BOOL refreshDisplay; //ˢ����Ļ��־λ 0 ��ˢ�� 1ˢ��
extern BOOL saveSetting;
extern unsigned char initFlag; //��ʼ����־λ
#endif

// =============added by leitiannet=============

// Ԥ�������40����ת��
#define ROTATIONANGLE_SIZE			40

// ������ת���ڴ������ж�Ӧ�ĵ�ַ
// ע��:���һ���ֱ�����0��ʼ(�����������±��Ӧ):0x0100-0x014E
#define SCREEN_ROTATIONANGLE_BEGIN	0x0100
#define SCREEN_ROTATIONANGLE_END	SCREEN_ROTATIONANGLE_BEGIN + (ROTATIONANGLE_SIZE * 2)

// �����ʶ
typedef enum
{
	MOTORFLAG_UNKOWN = 0,
	MOTORFLAG_ONE,
	MOTORFLAG_TWO,
	MOTORFLAG_ALL
} MotorFlag;

// ���λ��
typedef enum
{
	MOTORPOSITION_BEGIN = 1,
	MOTORPOSITION_END	= ROTATIONANGLE_SIZE
} MotorPosition;

// �������
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

// ���Ƶ��
void DisablePulse();
void ChangePulse();
void FinishPulse();
void HandleMotor(int addr, int dat);
void InitMotor();
void ControlMotor(int direction);
void StartSecondMotor();

void TestMotor();

void UserControl();

// ���ò���---keyΪ������Ӧ�ĵ�ַ
void ResetParameters();
void InitParameters();
void SetParameter(int key, int dat);
void SaveParameters();
void DisplayParameters();
void UpdateParameters();
void SetRotationAngle(int key, int dat);
void SyncRotationAngleNumber();
void SyncRotationAngle();

// �������ڷ��͹���������
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
