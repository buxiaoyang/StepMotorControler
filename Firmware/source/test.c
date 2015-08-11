#include "test.h"
#include "basefunc.h"
#include "parameter.h"
#include "screen.h"
#include "uart.h"

void Test_EEPROM()
{
	BOOL ret = TRUE;
	SendString("Test EEPROM\r\n");
	SaveParametersToEEPROM();
	SendString("1.Read parameters\r\n");
	ret = ReadParametersFromEEPROM();
	if (!ret)
	{
		SendString("Read parameters error\r\n");
		return;
	}
	LogParameters();
	SendString("2.Change parameters and save Parameters\r\n");
	ChangeParameters();
	SaveParametersToEEPROM();
	SendString("3.Read parameters again\r\n");
	ret = ReadParametersFromEEPROM();
	if (!ret)
	{
		SendString("Read parameters error\r\n");
		return;
	}
	LogParameters();
	SendString("Test EEPROM success\r\n");
}
