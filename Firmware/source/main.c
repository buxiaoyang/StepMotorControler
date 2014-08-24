/***************************************************************************/
// ������ϴ�豸������
// ģ�飺������	
// �ļ���main.c
// ���ߣ������D
// �汾��V0.02
// ���ڣ�2013��11��8��
// ���ܣ�������
// оƬ��STC12C5A60S2
// ���룺Keil uVision3 V3.90
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
	//��ʼλ�ô�����
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
	//��ʼλ�ô�����2
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
	//���������
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
// ������
// ��������
// ����ֵ����	
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
		//����ɨ��
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
		//��ʼ���������ж�ԭ��
		if(initFlag == 1 && isStartPosition == 1)
		{
			pulseSettingNumCount = 4;
			currentPosition = 1;
			initFlag = 0;
		}
		//Save setting to eeprom
		if(saveSetting)
		{
			//��������ת��
			temp = (unsigned long)pulseSettingNum*motorStepAngle;
			motorRotationAngle = temp/motorReducGearRatio;
			//����˿�˵���
			temp = (unsigned long)pulseSettingNum*motorStepAngle;
			temp = 	(unsigned long)temp * screwPitch;
			temp = 	(unsigned long)temp / motorReducGearRatio;
			ballScrew = temp/3600;
			//��������
			ChangeScreenPage(0x04);
			parameter_save();
			ChangeScreenPage(0x02);
			saveSetting = 0;
			refreshDisplay = 1;
		}
		//ˢ����ʾ	
		if(refreshDisplay)
		{
			parameter_send_screen();
			refreshDisplay = 0;
		}
	}   
}








