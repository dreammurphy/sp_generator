// DvsEventGen.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include<unistd.h>
#endif

//#define HAS_ASYNC_THREAD

//-----------------------------------
// debug
//-----------------------------------

//#define DEBUGLOG

#ifdef DEBUGLOG
#define DbgPrint(fmt, ...)	printf(fmt, __VA_ARGS__)
#else
#define DbgPrint(fmt, ...) 
#endif

//----------------------------------
//Endian
#define SWAP_ENDIAN
#define SWAP32(x) ( ((x & 0x000000FF) << 24)| \
					((x & 0x0000FF00) << 8)| \
					((x & 0x00FF0000) >> 8)| \
					((x & 0xFF000000) >> 24) )

#define QUEUE_SIZE	(128)
// --------------------------------------
// struct 
// --------------------------------------
typedef struct str_dvs_event
{
	int    x;
	int    y;
	int    polarity; //-1: intensity weakened; 1: intensity is increased; 0 intensity unchanged
	int    time;
} STR_DVS_EVENT;

// --------------------------------------
// function declaration
// --------------------------------------
// step1: init
// step2: loop get_dvs_event
// step3: destroy when want to exit
void init_dvsEventGen(char* filename, bool bloop);
void destroy_dvsEventGen(void);

void get_dvs_event(struct str_dvs_event* p_dvs_data, int* flag);

