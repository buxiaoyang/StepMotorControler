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
		if(isStartPosition == 0)
		{
			refreshDisplay = 1;
		}
		isStartPosition = 1;
	}
	if(sensorStartPosi1 == 1)
	{
		if(isStartPosition == 1)
		{
			refreshDisplay = 1;
		}
		isStartPosition = 0;
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
	delay_ms(500);
	parameter_init();
	uart_init();
	timer_init();
	while(1)
	{
		delay_us(100);
		if(pulseSettingNumCount > 150)
		{
			timer_count --;
		}
		else
		{
			timer_count ++;
		}
		if(timer_count < 12)
		{
			timer_count = 12;	
		}
		else if(timer_count > 50)
		{
			timer_count = 50;
		}
		getSensorStatus();
		if(saveSetting)
		{
			ChangeScreenPage(0x04);
			parameter_save();
			ChangeScreenPage(0x02);
			saveSetting = 0;
		}
		//ˢ����ʾ	
		if(refreshDisplay)
		{
			parameter_send_screen();
			refreshDisplay = 0;
		}
	}   
}








