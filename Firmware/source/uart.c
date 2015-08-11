#include "uart.h"
//#include <reg52.h>
#include <intrins.h>
#include <parameter.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FOSC 11059200L      //System frequency
#define BAUD 115200         //UART baudrate

/*Define UART parity mode*/
#define NONE_PARITY     0   //None parity
#define ODD_PARITY      1   //Odd parity
#define EVEN_PARITY     2   //Even parity
#define MARK_PARITY     3   //Mark parity
#define SPACE_PARITY    4   //Space parity

#define PARITYBIT NONE_PARITY   //Testing even parity

#if 0
/*Declare SFR associated with the UART2 */
sfr AUXR  = 0x8e;           //Auxiliary register
sfr S2CON = 0x9a;           //UART2 control register
sfr S2BUF = 0x9b;           //UART2 data buffer
sfr BRT   = 0x9c;           //Baudrate generator
sfr IE2   = 0xaf;           //Interrupt control 2
#endif

#define S2RI  0x01          //S2CON.0
#define S2TI  0x02          //S2CON.1
#define S2RB8 0x04          //S2CON.2
#define S2TB8 0x08          //S2CON.3

bit busy;
bit uartReceiveOK = 0;
//BYTE saveSetting = 0;

BYTE uartBuffer[15] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

BYTE receiveSteps = 0; 	// 0: û�����ݽ���
						// 1: ���յ�5A
						// 2�����յ�A5
						// 3�����յ����ݳ���
						// 4: ��������
						// 5��ȷ��ָ���
BYTE dataIndex = 0;
BYTE dataLength = 0;
  
void SendData(BYTE dat);
void SendString(char *s);
void ReceiveData(BYTE dat);
void anyData();

void uart_init()
{
#if (PARITYBIT == NONE_PARITY)
    S2CON = 0x50;           //8-bit variable UART
#elif (PARITYBIT == ODD_PARITY) || (PARITYBIT == EVEN_PARITY) || (PARITYBIT == MARK_PARITY)
    S2CON = 0xda;           //9-bit variable UART, parity bit initial to 1
#elif (PARITYBIT == SPACE_PARITY)
    S2CON = 0xd2;           //9-bit variable UART, parity bit initial to 0
#endif

    BRT = -(FOSC/32/BAUD);  //Set auto-reload vaule of baudrate generator
    AUXR |= 0x14;            //Baudrate generator work in 1T mode
    IE2 = 0x01;             //Enable UART2 interrupt
    EA = 1;                 //Open master interrupt switch

}

void Uart2InterruptReceive(void) interrupt 8
{
	unsigned char tmp = 0;
	char i = 0;

	IE2 &= (~ES2);//�رմ���2�ж�//ES2 = 0;
	if (S2CON & S2RI)	//if(S2RI == 1)
	{

		S2CON &= (~S2RI);//S2RI = 0;
		tmp = S2BUF;
		ReceiveData(tmp);
	}
	else
	{
		S2CON &= (~S2TI);//S2TI = 0;
	}

	IE2 |= ES2;//ES2 = 1;
}

void SendData(char i)
{
	unsigned char temp = 0;
	
	IE2 &= (~ES2);//�رմ���2�ж�//ES2 = 0;
	S2CON &= (~S2TI);//S2TI = 0;
	S2BUF = i;//װ������
	do
	{
		temp = S2CON;
		temp = temp & 0x02;//temp = S2TI;
	}while(temp == 0);//�ж��Ƿ������

	S2CON &= (~S2TI);//S2TI = 0;
	IE2 |= ES2;//ES2 = 1;
		
}

void SendString(char *data_at)
{
	unsigned char cnt=0;

	IE2 &= (~ES2);//�رմ���2�ж�//ES2 = 0;
	S2CON &= (~S2TI);//S2TI = 0;		
	while(*(data_at+cnt))	//�ж�һ�������Ƿ����
	{
		S2BUF = *(data_at+cnt);//װ������
		while((S2CON & S2TI) == 0);
		S2CON &= (~S2TI);//S2TI = 0;
		cnt++;

	}

	S2CON &= (~S2TI);//S2TI = 0;
	IE2 |= ES2;//ES2 = 1;		
}


void SendBuffer(char *buf, int len)
{
	int i = 0;
	for (i = 0; i < len; i++)
	{
		SendData(buf[i]);
	}
}

// name=value
void SendKeyValue(char *name, int value)
{
	int len = 0;
	char dataBuf[50] = {0};
	len = sprintf(dataBuf, "%s=%d\r\n", name, value);
	SendBuffer(dataBuf, len);
}

void ReceiveData(BYTE dat)
{
	// 0: ���յ�5A
	// 1�����յ�A5
	// 2�����յ����ݳ���
	// 3: ��������
	// 4��ȷ��ָ���
	switch(receiveSteps)
	{
		case 0:
			if(dat == 0x5A)
			{
				receiveSteps ++;
			}
		break;
		case 1:
			if(dat == 0xA5)
			{
				receiveSteps ++;
			}
			else
			{
			  	receiveSteps = 0;
			}
		break;
		case 2:
			dataLength = dat;
			dataIndex = 0;
			receiveSteps ++;
		break;
		case 3:
			uartBuffer[dataIndex] = dat;
			dataIndex ++;
			if(dataIndex >= dataLength)
			{
				anyData();
				receiveSteps = 0;
			}
		break;
		default:
			_nop_();
			receiveSteps = 0;
	}
}

void anyData()
{
	ParseCommand(uartBuffer, sizeof(uartBuffer));
	uartReceiveOK = 1;	
}


/*////////////////////////////////////////////////////////////////////////////////////////////////////
// ������:char Uart1Init(char smod,char brtx12,unsigned char reload)
// ����:	��ʼ����������gps�Ĵ���1 ,������1����λ���������ʷ������ṩ�����ʣ�
			���򿪴���1�жϺ�ȫ���ж�
// ����:	smod	smodλ	0/1
			brtx12	brtx12λ  0/1
			reload	reload�Ĵ�����ֵ  0-255
// ����:	-1		smodλ����û��д��0/1
			-2		brtx12λ����û��д��0/1
			0		
////////////////////////////////////////////////////////////////////////////////////////////////////*/
//��������λ�����������ṩ�����ʣ��ʹ���2ʹ��ͬһ�������ʣ�Ҳ����ʹ�ö�ʱ��1���ṩ������
char Uart1Init(char smod,char brtx12,unsigned char reload)
{
	SCON =  0X50;//8λ�ɱ䲨���ʣ�����żλ(SM0=0,SM1=1),ʹ�ܴ��ڽ���ģ��(REN=1)
	BRT	= reload;//���ö��������ʷ�����������

	if(smod == 1)
	{
		PCON |= SMOD;   //SMOD = 1;//�����ʱ���λ
	}
	else if(smod == 0)
	{
		PCON &= (~SMOD);   //SMOD = 0;//ȡ�������ʱ���λ
	}
	else
	{
		return -1;
	}
		
	if(brtx12 == 1)
	{
		AUXR |= BRTx12;//BRTx12 = 1;1Tģʽ	
	}
	else if(brtx12 == 0)
	{
		AUXR &= (~BRTx12);//BRTx12 = 0;12Tģʽ		
	}
	else
	{
		return -2;
	}
	
  	AUXR |= S1BRS;//����1����Ϊʹ�ö��������ʷ�����										
	AUXR |= BRTR;//���������ʷ�����									
											
	ES = 1;	   //�������ж�
	EA = 1;	   //�����ж�
	return 0;	
}

/*////////////////////////////////////////////////////////////////////////////////////////////////////
// ������:void Uart1Send(char i)
// ����: ����gps���ӵĴ���1��gps����1�ֽ�����
// ����:	i	Ҫ���͵�����	
// ����:	 void
////////////////////////////////////////////////////////////////////////////////////////////////////*/
void Uart1Send(char i)
{
	ES = 0; //�ش����ж�
	TI = 0; //��շ������ж������־λ
	SBUF = i;  //�����ݷ���Ĵ�������
	while(TI == 0);//�ȴ�������ϣ�������� TI == 1
	TI = 0;	//��շ������ж������־λ
	ES = 1;	 //�������ж�	
}

/*////////////////////////////////////////////////////////////////////////////////////////////////////
// ������:void Uart1Sends(char* at)
// ����: �����ַ���������1
// ����: char* at �ַ���ͷ��ַ
// ����:
////////////////////////////////////////////////////////////////////////////////////////////////////*/
void Uart1Sends(char* at)
{
	unsigned char cnt=0;

	ES=0;//�ش��п��ж�	
	while(*(at+cnt))	//�ж�һ�������Ƿ����
	{
		SBUF=*(at+cnt);	//��������
		while(TI==0);	//��ѯ�����Ƿ����
		TI=0;	//�������һ��־λ
		cnt++;	//׼������һ������
	}
	ES=1;//�����п��ж�		
}
