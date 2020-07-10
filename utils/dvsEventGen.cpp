// DvsEventGen.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "DvsEventGen.h"

#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif

static bool bThreadExit = false;

#ifdef _WIN32
CRITICAL_SECTION queuelock;
#else

#endif

struct param_dvseg {
	char filename[128];
	bool bloop;
};

#ifndef HAS_ASYNC_THREAD
struct param_dvseg gctx_param;

FILE* fp;
int curevnetcnt = 0;
int eventnum = 0;

void init_dvsEventGen(char* filename, bool bloop)
{
	//transfer the param
	strcpy_s(gctx_param.filename, filename);
	gctx_param.bloop = bloop;

	//open the file
	bool ret = 0;


	//open file and skip the head info
	fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		printf("Err: file %s can not open\n", filename);
		return;
	}

	//read file
	fseek(fp, 0x0, SEEK_END);
	eventnum = (ftell(fp) - 0xD8) / 8;
	printf("Total event number = %d\n", eventnum);

	//remind and skip the header section
	fseek(fp, 0xD8, SEEK_SET);
}

void destroy_dvsEventGen(void)
{
	//close file
	if (fp)
	{
		fclose(fp);
	}
}

void get_dvs_event(struct str_dvs_event* p_dvs_data, int* flag)
{
	unsigned long addr, ts;

	//check file end?
	if (curevnetcnt >= eventnum)
	{
		if (gctx_param.bloop == true)
		{
			//replay
			curevnetcnt = 0;
			fseek(fp, 0xD8, SEEK_SET);
		}
		else
		{	//not reloop and exit
			*flag = 0;
			return;
		}
	}

	//read the data
	fread(&addr, sizeof(long), 1, fp);
	fread(&ts, sizeof(long), 1, fp);

#ifdef SWAP_ENDIAN
	addr = SWAP32(addr);
	ts = SWAP32(ts);
#endif
	//x 7:1
	//y 14:8
	//p 0
	p_dvs_data->x = ((addr & 0x00FE) >> 1);
	p_dvs_data->y = ((addr & 0x7F00) >> 8);
	p_dvs_data->polarity = (addr & 0x0001);
	p_dvs_data->time = ts;

	//debug
	//printf("queue_wpos=%d queue_restcnt=%d\n", queue_wpos, queue_restcnt);
	DbgPrint("[%04d]x=%d y=%d p=%d ts=%d\n",
		curevnetcnt,
		p_dvs_data->x,
		p_dvs_data->y,
		p_dvs_data->polarity,
		p_dvs_data->time);

	//Event plus+1
	curevnetcnt++;
}

#else

//--------------------------------------------------
// function declaration
unsigned int __stdcall DvsAedatDataReadProc(LPVOID lpParam);

STR_DVS_EVENT* gQueueData;
int queue_rpos = 0;
int queue_wpos = 0;
int queue_restcnt = 0;

void init_dvsEventGen(char *filename, bool bloop)
{
	HANDLE hThread;
	unsigned int threadid = 0;
	
	struct param_dvseg  *ctx_param;

	//transfer the param
	ctx_param = (struct param_dvseg *)malloc(1 * sizeof(struct param_dvseg));
	strcpy(ctx_param->filename, filename);
	ctx_param->bloop = bloop;

	//alloc the queue
	gQueueData = (STR_DVS_EVENT*)malloc(QUEUE_SIZE * sizeof(STR_DVS_EVENT));

#ifdef _WIN32
	InitializeCriticalSection(&queuelock);
	//Create thread
	hThread = (HANDLE)_beginthreadex(NULL, 0, DvsAedatDataReadProc, ctx_param, 0, &threadid);
#else

#endif
}

void destroy_dvsEventGen(void)
{
	printf("Enter destroy_dvsEventGen\n");

	bThreadExit = true;
	DeleteCriticalSection(&queuelock);
}

unsigned int __stdcall DvsAedatDataReadProc(LPVOID lpParam)
{
	bool ret = 0;
	FILE *fp;
	unsigned int tret = 0;

	int curevnetcnt = 0;
	int eventnum = 0;

	unsigned long addr, ts;

	struct param_dvseg* pctx_param = (struct param_dvseg*)lpParam;

	//open file and skip the head info
	fp = fopen( pctx_param->filename, "rb");
	if (fp == NULL)
	{	
		printf("Err: file %s can not open\n", pctx_param->filename);
		_endthreadex(tret);
		return ret;
	}

	//read file
	fseek(fp, 0x0, SEEK_END);
	eventnum = (ftell(fp) - 0xD8) / 8;
	printf("Total event number = %d\n", eventnum);

	//remind and skip the header section
	fseek(fp, 0xD8, SEEK_SET);

	while (true)
	{
#ifdef _WIN32
		Sleep(1);		//1ms
#else
		usleep(1000);
#endif

		if (queue_restcnt >= QUEUE_SIZE)
		{	//full
			continue;
		}

		//read the data
		fread(&addr, sizeof(long), 1, fp);
		fread(&ts, sizeof(long), 1, fp);

#ifdef SWAP_ENDIAN
		addr = SWAP32(addr);
		ts = SWAP32(ts);
#endif
		//x 7:1
		//y 14:8
		//p 0
		gQueueData[queue_wpos].x = ((addr & 0x00FE) >> 1);
		gQueueData[queue_wpos].y = ((addr & 0x7F00) >> 8);
		gQueueData[queue_wpos].polarity = (addr & 0x0001);
		gQueueData[queue_wpos].time = ts;

		//debug
		//printf("queue_wpos=%d queue_restcnt=%d\n", queue_wpos, queue_restcnt);
		DbgPrint("[%04d]x=%d y=%d p=%d ts=%d\n",
			curevnetcnt,
			gQueueData[queue_wpos].x,
			gQueueData[queue_wpos].y,
			gQueueData[queue_wpos].polarity,
			gQueueData[queue_wpos].time);

		//queue next
		EnterCriticalSection(&queuelock);
		queue_restcnt++;
		LeaveCriticalSection(&queuelock);
		queue_wpos++;
		if (queue_wpos >= QUEUE_SIZE)
		{
			queue_wpos = 0;
		}

		//Event plus+1
		curevnetcnt++;
		if (curevnetcnt >= eventnum)
		{
			if (pctx_param->bloop == true)
			{
				//replay
				curevnetcnt = 0;
				fseek(fp, 0xD8, SEEK_SET);
			}
			else
			{	//not reloop and exit
				bThreadExit = true;
			}
		}

		//check if exit or not
		if (bThreadExit)
		{
			break;
		}
	}//..while true

	// _endthread given to terminate
	_endthreadex(tret);

	return ret;
}

void get_dvs_event(struct str_dvs_event* p_dvs_data, int* flag)
{
	//printf("Enter get_dvs_event\n");

	if ((queue_restcnt == 0)||(bThreadExit==true))
	{	//empty
		*flag = 0;
		return;
	}

	memcpy(p_dvs_data, (void *)(&gQueueData[queue_rpos]), sizeof(STR_DVS_EVENT));
	*flag = 1;

	EnterCriticalSection(&queuelock);
	queue_restcnt--;
	LeaveCriticalSection(&queuelock);
	queue_rpos++;
	if (queue_rpos >= QUEUE_SIZE)
	{
		queue_rpos = 0;
	}
}

#endif
