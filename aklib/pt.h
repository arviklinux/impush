/***************************************
*file: 			pt.h
*description:	for print debug info
*noted:			support by c99
*author:		arvik
*blog:			http://blog.csdn.net/u012819339
*email:			1216601195@qq.com
*
* kernel: linux-3.10.36
***************************************/
#pragma once

#include <stdio.h>
/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

/****************pt_debug setting*****************/
//需要在gcc编译时作为宏传入
#ifdef PT_LEVEL1
	#define PT_LEVEL   1
#endif

#ifdef PT_LEVEL2
	#define PT_LEVEL   2
#endif

#ifdef PT_LEVEL3
	#define PT_LEVEL   3
#endif

#ifdef PT_LEVEL4
	#define PT_LEVEL   4
#endif

#ifdef PT_LEVEL5
	#define PT_LEVEL   5
#endif

//默认打印等级
#ifndef PT_LEVEL
	#define PT_LEVEL	3
#endif



#define PT_DEBUG	5
#define PT_INFO		4
#define PT_WARN		3
#define PT_ERROR	2
#define PT_FATAL	1


/*********************color setting****************/
/*********************
printf("\033[字体背景颜色;字体颜色m字符串\033[0m");
控制字符通用格式：
Esc[{attr1};...;{attrn}m
其中：
Esc	是转义字符，其值为"\033"
[	是常量的左中括号
{attr1};...{attrn}	是若干属性，通常是由一个有特定意义的数字代替, 每个属性之间用分号分隔
m	就是字面常量字符m

颜色代码：
字背景颜色范围：40-49				字颜色：30-39
				40：黑						30：黑
				41：红						31：红
				42：绿						32：绿
				43：黄						33：黄
				44：蓝						34：蓝
				45：紫						35：紫
				46：深绿					36：深绿
				47：白色					37：白色
				
ANSI控制码：
\033[0m   重置所有属性  
\033[1m   高亮/加粗  
\033[2m   暗淡
\033[4m   下划线  
\033[5m   闪烁  
\033[7m   反显/反转  
\033[8m   消隐/隐藏  

\033[30m   --   \033[37m   设置前景色  
\033[40m   --   \033[47m   设置背景色  
\033[nA   光标上移n行  
\033[nB   光标下移n行  
\033[nC   光标右移n行  
\033[nD   光标左移n行  
\033[y;xH设置光标位置  
\033[2J   清屏  
\033[K   清除从光标到行尾的内容  
\033[s   保存光标位置  
\033[u   恢复光标位置  
\033[?25l   隐藏光标  
\33[?25h   显示光标
**************************/

#define C_RESET	"\033[0m"  //关闭所有属性

#define FB_RG	"\033[44;31m" //蓝色背景，红色字体  ---Fatal
#define F_C_RED	"\033[0;31m" //红字体				---error
#define F_C_BLUE	"\033[0;34m"  //蓝字体			---warn
#define F_C_GREEN	"\033[0;32m"  //绿色字体		---info
#define F_BOLD		"\033[1m"	  //加粗	     	---debug

#define B_C_RED	"\033[41m" 


/*********标准宏定义************************
__LINE__：在源代码中插入当前源代码行号；
__FILE__：在源文件中插入当前源文件名；
__DATE__：在源文件中插入当前的编译日期
__TIME__：在源文件中插入当前编译时间；
__STDC__：当要求程序严格遵循ANSI C标准时该标识被赋值为1；
__cplusplus：当编写C++程序时该标识符被定义。
********************************************/

//debug 行号	加粗
#if PT_DEBUG <= PT_LEVEL
	#define PT_Debug(format,...) printf(F_BOLD"line:%05d:"format C_RESET, __LINE__, ##__VA_ARGS__)  
#else  
	#define PT_Debug(format,...)  
#endif 

//info 行号
#if PT_INFO <= PT_LEVEL
	#define PT_Info(format,...) printf(F_C_GREEN"line:%05d:"format C_RESET, __LINE__, ##__VA_ARGS__)
#else
	#define PT_Info(format,...)
#endif

//warn 行号、函数
#if PT_WARN <= PT_LEVEL
	#define PT_Warn(format,...) printf(F_C_BLUE"line:%05d||func:%s:\n"format C_RESET, __LINE__,  __FUNCTION__, ##__VA_ARGS__)
#else
	#define PT_Warn(format,...)
#endif

//error
#if PT_ERROR <= PT_LEVEL
	#define PT_Error(format,...) do{ \
	printf(F_C_RED"file:%s||line:%05d||func:%s:\n"format C_RESET, __FILE__, __LINE__,  __FUNCTION__, ##__VA_ARGS__); \
	}while(0) 
#else
	#define PT_Error(format,...)
#endif

//fatal
#if PT_FATAL <= PT_LEVEL
	#define PT_Fatal(format,...) do{ \
	printf(FB_RG"file:%s||line:%05d||func:%s:\n"format C_RESET, __FILE__, __LINE__,  __FUNCTION__, ##__VA_ARGS__); \
	}while(0)
#else
	#define PT_Fatal(format,...)
#endif


/***********************同行打印信息*************************/
//同行 加粗
#define PT_SL(format,...) do{ printf("\r\033[K;1m"format"\r"C_RESET, ##__VA_ARGS__); fflush(stdout); }while(0)  

/************普通加粗打印*********************/
#define PT_B(format,...) printf(F_BOLD format C_RESET, ##__VA_ARGS__)

/*********************红色醒目打印*********************/
#define PT_RED(format,...)	printf(F_C_RED format C_RESET, ##__VA_ARGS__)




