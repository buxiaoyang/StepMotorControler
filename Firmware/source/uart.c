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

BYTE receiveSteps = 0; 	// 0: 没有数据接收
						// 1: 接收到5A
						// 2：接收到A5
						// 3：接收到数据长度
						// 4: 接收数据
						// 5：确定指令动作
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

	IE2 &= (~ES2);//关闭串口2中断//ES2 = 0;
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
	
	IE2 &= (~ES2);//关闭串口2中断//ES2 = 0;
	S2CON &= (~S2TI);//S2TI = 0;
	S2BUF = i;//装入数据
	do
	{
		temp = S2CON;
		temp = temp & 0x02;//temp = S2TI;
	}while(temp == 0);//判断是否发送完毕

	S2CON &= (~S2TI);//S2TI = 0;
	IE2 |= ES2;//ES2 = 1;
		
}

void SendString(char *data_at)
{
	unsigned char cnt=0;

	IE2 &= (~ES2);//关闭串口2中断//ES2 = 0;
	S2CON &= (~S2TI);//S2TI = 0;		
	while(*(data_at+cnt))	//判断一串数据是否结束
	{
		S2BUF = *(data_at+cnt);//装入数据
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
	// 0: 接收到5A
	// 1：接收到A5
	// 2：接收到数据长度
	// 3: 接收数据
	// 4：确定指令动作
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
// 函数名:char Uart1Init(char smod,char brtx12,unsigned char reload)
// 作用:	初始化用于连接gps的串口1 ,将串口1设置位独立波特率发生器提供波特率，
			并打开串口1中断和全局中断
// 参数:	smod	smod位	0/1
			brtx12	brtx12位  0/1
			reload	reload寄存器数值  0-255
// 返回:	-1		smod位错误，没有写入0/1
			-2		brtx12位错误，没有写入0/1
			0		
////////////////////////////////////////////////////////////////////////////////////////////////////*/
//串口设置位独立波特率提供波特率，和串口2使用同一个波特率，也可以使用定时器1来提供波特率
char Uart1Init(char smod,char brtx12,unsigned char reload)
{
	SCON =  0X50;//8位可变波特率，无奇偶位(SM0=0,SM1=1),使能串口接收模块(REN=1)
	BRT	= reload;//设置独立波特率发生器波特率

	if(smod == 1)
	{
		PCON |= SMOD;   //SMOD = 1;//波特率倍速位
	}
	else if(smod == 0)
	{
		PCON &= (~SMOD);   //SMOD = 0;//取消波特率倍速位
	}
	else
	{
		return -1;
	}
		
	if(brtx12 == 1)
	{
		AUXR |= BRTx12;//BRTx12 = 1;1T模式	
	}
	else if(brtx12 == 0)
	{
		AUXR &= (~BRTx12);//BRTx12 = 0;12T模式		
	}
	else
	{
		return -2;
	}
	
  	AUXR |= S1BRS;//串口1设置为使用独立波特率发生器										
	AUXR |= BRTR;//开启波特率发生器									
											
	ES = 1;	   //开串口中断
	EA = 1;	   //开总中断
	return 0;	
}

/*////////////////////////////////////////////////////////////////////////////////////////////////////
// 函数名:void Uart1Send(char i)
// 作用: 用于gps连接的串口1向gps发送1字节数据
// 参数:	i	要发送的数据	
// 返回:	 void
////////////////////////////////////////////////////////////////////////////////////////////////////*/
void Uart1Send(char i)
{
	ES = 0; //关串口中断
	TI = 0; //清空发送完中断请求标志位
	SBUF = i;  //将数据放入寄存器发送
	while(TI == 0);//等待发送完毕，发送完毕 TI == 1
	TI = 0;	//清空发送完中断请求标志位
	ES = 1;	 //开串口中断	
}

/*////////////////////////////////////////////////////////////////////////////////////////////////////
// 函数名:void Uart1Sends(char* at)
// 作用: 发送字符串到串口1
// 参数: char* at 字符串头地址
// 返回:
////////////////////////////////////////////////////////////////////////////////////////////////////*/
void Uart1Sends(char* at)
{
	unsigned char cnt=0;

	ES=0;//关串行口中断	
	while(*(at+cnt))	//判断一串数据是否结束
	{
		SBUF=*(at+cnt);	//发送数据
		while(TI==0);	//查询发送是否结束
		TI=0;	//清除发送一标志位
		cnt++;	//准备发送一个数据
	}
	ES=1;//开串行口中断		
}
