/***************************************************************************/
// ������ϴ�豸������
// ģ�飺ͨ�ú���ͷ�ļ�	
// �ļ���basefunc.h
// ���ߣ������D
// �汾��V0.02
// ���ڣ�2013��11��8��
// ���ܣ�ͨ�ú���ͷ�ļ�
// оƬ��STC12C5A60S2
// ���룺Keil uVision3 V3.90
/***************************************************************************/

#ifndef __BASEFUNC_H__
#define __BASEFUNC_H__
#include "stc12c5a.h"

/*
typedef unsigned char	BOOLEAN;
typedef unsigned char	INT8U;   
typedef signed   char	INT8S;                   
typedef unsigned int 	INT16U;                   
typedef signed   int 	INT16S;                   
typedef unsigned long   INT32U;                   
typedef signed   long   INT32S;                  
typedef float          	FP32;                    
typedef double         	FP64;  
*/

typedef unsigned char BYTE;
typedef unsigned int  WORD;
typedef unsigned long LONG;
typedef unsigned char BOOL;
typedef void          VOID;

#define TRUE		  1
#define FALSE		  0

void _nop_ (void);

/***************************************************************************/
// ��������
/***************************************************************************/
void delay_us(WORD us_value);
void delay_ms(WORD ms_value);

#endif
