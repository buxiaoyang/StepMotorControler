/***************************************************************************/
// ������ϴ�豸������
// ģ�飺����
// �ļ���parameter.c
// ���ߣ������D
// �汾��V0.02
// ���ڣ�2013��11��8��
// ���ܣ�����
// оƬ��STC12C5A60S2
// ���룺Keil uVision3 V3.90
/***************************************************************************/

#include <parameter.h>
#include <eeprom.h>
#include <basefunc.h>

/***************************************************************************/
// ��������	
/***************************************************************************/
unsigned char currentPosition; //��ǰλ�� 1~20
unsigned int pulseSettingNum; //�������
unsigned int pulseSettingFreq; //����Ƶ��
unsigned int motorStepAngle; //���������
unsigned int screwPitch; //˿��˿��
unsigned int motorReducGearRatio; //������ٱ�
unsigned int ballScrew; //˿�˵���
unsigned int motorRotationAngle; //�����ת��
unsigned char isStartPosition; //��ʼλ��
unsigned char isStartPosition1; //��ʼλ��
unsigned char isStartPosition2; //��ʼλ��

unsigned char refreshDisplay; //ˢ����Ļ��־λ 0 ��ˢ�� 1ˢ��

unsigned char initFlag; //��ʼ����־λ

/***************************************************************************/
// ��ȡ����
// ��������
// ����ֵ��0ʧ�� 1�ɹ�	
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
// ������ʼ������
// ��������
// ����ֵ����	
/***************************************************************************/
void parameter_init()
{
	currentPosition = 1; //��ǰλ�� 1~20
	if(!parameter_read())
	{
		pulseSettingNum = 400; //�������
		pulseSettingFreq = 400; //����Ƶ��
		motorStepAngle = 180; //���������
		screwPitch = 200; //˿��˿��
		motorReducGearRatio = 100; //������ٱ�
		ballScrew = 200; //˿�˵���
		motorRotationAngle = 36000; //�����ת��
	}
	isStartPosition = 0; //��ʼλ��
	refreshDisplay = 1;
	initFlag = 0;
}

/***************************************************************************/
// �������浽eeprom
// ��������
// ����ֵ��0ʧ�� 1�ɹ�	
/***************************************************************************/
unsigned char parameter_save()
{
	
	unsigned char result = 1;
    delay_ms(10);                      //Delay
	IapEraseSector(IAP_ADDRESS); //����EEPROM
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
	IapProgramByte(IAP_ADDRESS+200, 0xEE); //д���־λ
	refreshDisplay = 0;
	return result;
}


