#include "debug.h"

#ifdef URAT1_DEBUG
	#include "uart.h"
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	
	char dataBuf[100] = {0};
	void InitDebug()
	{
		Uart1Init(0, 1, 253);//初始化串口，设置波特率115200
	}

	void DebugFunction(char* function, int addr, int dat)
	{

		if (dat == 0x00EE)
		{
			sprintf(dataBuf, "%s addr=%-#4X dat=%-#4X\r\n", function, addr, dat);
		}
		else
		{
			sprintf(dataBuf, "%s addr=%-#4X dat=%d\r\n", function, addr, dat);
		}
		Uart1Sends(dataBuf);
	}

	void DebugParameter(char *name, long value)
	{
		sprintf(dataBuf, "%s=%ld\r\n", name, value);
		Uart1Sends(dataBuf);
	}
	
	void DebugParameter1(int index, long value)
	{
		sprintf(dataBuf, "%-02d:%d\r\n", index, value);
		Uart1Sends(dataBuf);
	}

	void Debug(char* str)
	{
		Uart1Sends(str);
	}
#endif 
