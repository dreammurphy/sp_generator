// DvsEventGen.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "DvsEventGen.h"
#include "ParseOpt.h"

char gfilepath[128];
int gbLoop = false;

static void parse_opts(int argc, char* argv[])
{
//	char* arg = NULL;
	int cmd = 0;
	int opt_index = -1;

	//------------------------------
	//loop parse
	do{
		cmd = getopt_long(argc, argv, lopts, opt_index);

		switch (cmd)
		{
		case ARG_PATH:
			strcpy_s(gfilepath, argv[opt_index - 1]);
			break;
		case ARG_LOOP:
			gbLoop = atoi(argv[opt_index - 1]);
			break;
		case ARG_HELP:
		default:
			if(opt_index!=argc)
			{
				printf("!!Unsupport command param: %s\n", argv[opt_index]);
				print_usage();
			}
			break;
		}
	}while (cmd != ARG_UNSUPPORT);
}

int main_4dvs(int argc, char* argv[])
{
//	bool ret;
	int  flag;
	STR_DVS_EVENT oneEvent;

//	int  count = 0;

	//parse the arguments
	parse_opts(argc, argv);

	//run the event data read operation
	init_dvsEventGen(gfilepath, gbLoop);

	//get the event one by one
	while(1)
	{
		get_dvs_event(&oneEvent, &flag);
		if (flag == 0)
		{	
			//the last one and quit
			break;
		}

		//dummy time interval
		Sleep(1);
	}

	//destroy
	destroy_dvsEventGen();

	return 0;
}
