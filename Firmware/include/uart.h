#ifndef __UART_H__
#define __UART_H__

#include "basefunc.h"

void uart_init();
void SendData(BYTE dat);
void SendString(char *s);
void SendBuffer(char *buf, int len);
void SendKeyValue(char *name, int value);
void anyData();

// 串口1用于调试
char Uart1Init(char smod, char brtx12, unsigned char reload);
void Uart1Send(char i);
void Uart1Sends(char* at);

#endif