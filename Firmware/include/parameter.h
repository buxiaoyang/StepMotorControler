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
sbit sensorStartPosi1 = P0^0; //��ʼλ�ô�����1
sbit sensorStartPosi2 = P0^1; //��ʼλ�ô�����2
sbit keyMotorForward = P0^2; //�����ǰ�ź�
sbit keyMotorBackward = P0^3; //�������ź�
sbit electromagnetIn = P0^4; //���������

//���
sbit motorDirection = P1^0; //�������
sbit electromagnetOut = P1^1; //��������
sbit motorPWM = P2^6; //�������

sbit testOut = P3^7;

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
extern unsigned char isStartPosition1; //��ʼλ��
extern unsigned char isStartPosition2; //��ʼλ��

extern unsigned char refreshDisplay; //ˢ����Ļ��־λ 0 ��ˢ�� 1ˢ��

extern unsigned char initFlag; //��ʼ����־λ
#endif
