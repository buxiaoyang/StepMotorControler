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

#include "basefunc.h"
#include "parameter.h"
#include "screen.h"
#include "uart.h"
#include <timer.h>
#include <key.h>
#include <dispatch.h>

#if 0
void PluseDelay(unsigned int a)
{
	while(a--);
}

void TestRun()
{
	/*
	�����������ת��һȦ
	�������������ϸ�ֱ���:8	����/Ȧ(1.8/����):1600
	ʵ�ʵĲ�����=1.8/8=0.225����תһȦ��Ҫ�������=360/0.225=1600
	*/
	unsigned int i = 0;
	secondMotorDirection = 0;
	for (i = 0; i < 1600; i++)
	{
		secondMotorPWM = 1;
		PluseDelay(800);
		secondMotorPWM = ~secondMotorPWM;
		PluseDelay(800);
	}
	while (1);
}
#endif

void main()
{
	WORD count10ms = 0;
	delay_ms(500);
	// ��ʼ��Ӳ��
	InitHardware();
    // ��ʼ������
	InitParameters();
	
	while (1)
	{
		delay_us(100);
		//����ɨ��---10us*30=300us=0.3ms,���ư���ɨ��Ƶ��
		count10ms++;
		if(count10ms > 29)
		{
			ScanKey();
			count10ms = 0;
		}

		AdjustTimerCount();
		CheckSensorInput();
		StartSecondMotor();
		DisplayParameters();
		SaveParameters();
	} 
}








