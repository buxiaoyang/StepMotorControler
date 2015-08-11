#include "key.h"
#include <intrins.h>
#include <parameter.h>
#include <key.h>
#include <dispatch.h>
#include <timer.h>

static unsigned char Key_Scan_Steps = 0; 		//0����ʼ״̬ ����а������������1
												//1���ӳ�10ms
												//2���������ȷʵ���������3
												//3��ȷ������
												//4����������ͷ������0

void ScanKey(void)
{
	//testOut = ~testOut;
	switch(Key_Scan_Steps)
	{
		case 0:
			if (keyMotorForward == 0 || keyMotorBackward == 0 ||
			    keyTestMotor == 0 || keyInitMotor == 0 || keyUserControl == 0)
			{
			   	Key_Scan_Steps = 1;
			}
		break;
		case 1:
			if (keyMotorForward == 0 || keyMotorBackward == 0 ||
			    keyTestMotor == 0 || keyInitMotor == 0  || keyUserControl == 0)
			{
			   	Key_Scan_Steps = 2;
			}
			else
			{
				Key_Scan_Steps = 0;
			}
		break;
		case 2:
			if (keyMotorForward == 0) //���ǰ��
			{
				ControlMotor(MOTORDIRECTION_FORWARD);
			}
			else if (keyMotorBackward == 0) //�������
			{
				ControlMotor(MOTORDIRECTION_BACKWARD);	
			}
			else if (keyTestMotor == 0)
			{
				TestMotor();
				//UserControl();
			}
			else if (keyInitMotor == 0)
			{
				InitMotor();
				//UserControl();
			}
			else if (keyUserControl == 0)
			{
				// ��Main���������Ƶ��������
				//StartSecondMotor();
				UserControl();
			}
			Key_Scan_Steps = 3;
			break;
		case 3:
			if (keyMotorBackward == 1 && keyMotorForward == 1 && 
			    keyTestMotor == 1 && keyInitMotor == 1 && keyUserControl == 1)
			{
			   	Key_Scan_Steps = 0;
			}
		break;
		default:
			 _nop_();
	}
}