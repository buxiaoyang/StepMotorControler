#include "screen.h"
#include "uart.h"
#include "parameter.h"

void SendLongDataToScreen(WORD address, LONG dat)
{
	SendData(0x5A);
	SendData(0xA5);
	SendData(0x07);
	SendData(0x82);
	SendData(address>>8);
	SendData(address);
	SendData((dat>>24) & 0xFF);
	SendData((dat>>16) & 0xFF);
	SendData((dat>>8) & 0xFF);
	SendData(dat & 0xFF);
}


void SendDataToScreen(WORD address, WORD dat)
{
	SendData(0x5A);
	SendData(0xA5);
	SendData(0x05);
	SendData(0x82);
	SendData(address>>8);
	SendData(address);
	SendData(dat>>8);
	SendData(dat);
}

void ChangeScreenPage(WORD page)
{
	SendData(0x5A);
	SendData(0xA5);
	SendData(0x04);
	SendData(0x80);
	SendData(0x03);
	SendData(page>>8);
	SendData(page);
	SetCurrentPage(page);
}


