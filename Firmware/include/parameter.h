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

#include <reg52.h>

/***************************************************************************/
// ���Ŷ���
/***************************************************************************/
//����
sbit sensorIn1 = P0^0; //������1
sbit sensorIn2 = P0^1; //������2
sbit sensorIn3 = P0^2; //������3
sbit sensorIn4 = P0^3; //������4
sbit sensorIn5 = P0^4; //������5
sbit sensorIn6 = P0^5; //������6
sbit sensorIn7 = P0^6; //������7
sbit sensorIn8 = P0^7; //������8
sbit sensorIn9 = P2^0; //������9
sbit sensorIn10 = P2^1; //������10

sbit KeyStart = P2^2; //��ʼ��ť
sbit KeyStop = P2^3; //��ͣ��ť

//���
sbit cylinderOut1 = P1^0; //����1
sbit cylinderOut2 = P1^1; //����2
sbit cylinderOut3 = P2^4; //����3
sbit cylinderOut4 = P2^5; //����4
sbit cylinderOut5 = P1^4; //����5
sbit montorOut = P1^5;		//������
sbit systemAlarmOut = P1^6; //ϵͳ����

sbit TestOut = P2^6;

/***************************************************************************/
// ��������
/***************************************************************************/
void parameter_init();
unsigned char parameter_save();

extern unsigned char currentPosition; //��ǰλ�� 1~20
extern unsigned int pulseSettingNum; //�������
extern unsigned int pulseSettingFreq; //����Ƶ��
extern unsigned int motorStepAngle; //���������
extern unsigned int screwPitch; //˿��˿��
extern unsigned int motorReducGearRatio; //������ٱ�
extern unsigned int ballScrew; //˿�˵���
extern unsigned int motorRotationAngle; //�����ת��
extern unsigned char isStartPosition; //��ʼλ��

extern unsigned char refreshDisplay; //ˢ����Ļ��־λ 0 ��ˢ�� 1ˢ��

#endif
