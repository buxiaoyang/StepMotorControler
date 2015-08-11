#ifndef __DEBUG_H__
#define __DEBUG_H__

// 使用串口1调试
//#define URAT1_DEBUG

#ifdef URAT1_DEBUG
	void InitDebug();
	void Debug(char* str);
	void DebugParameter(char *name, long value);
	void DebugParameter1(int index, long value);
	void DebugFunction(char* function, int addr, int dat);
#else
	#define InitDebug()
	#define Debug(str)
	#define DebugParameter(name, value)
	#define DebugParameter1(index, value)
	#define DebugFunction(function, addr, dat)
#endif

#endif
