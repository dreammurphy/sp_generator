
#include "dvs_process.h"

char g_dvs_in_name[] 	= "./data/mnist_0_scale04_0001.aedat";
char g_dvs_out_name[]  = "./data/dvs_test_data_out.txt";



dvs_processor_cla::dvs_processor_cla()
{
	dvs_processor_init();
}

dvs_processor_cla::~dvs_processor_cla()
{

}

void dvs_processor_cla::dvs_processor_init(void)
{
	pic_x = DVS_X_MAX;
	pic_y = DVS_Y_MAX;
	pic_c = DVS_RGB_MAX;
	pic_size_one = pic_x*pic_y;
	dvs_time_period 	= 1000;
	data_src 			= 0; // file
	dvs_event_max 		= 1000; // max number events to generate a frame, 200 etc.
	dvs_event_cnt 		= 0;
	dvs_time_start 		= 0;	
	data_scr_name[0]	= '\0';
	dvs_cont_fail_max 	= 3;
}

void dvs_processor_cla::dvs_pro_set_pic_size(int x, int y, int c)
{
	pic_x = x;
	pic_y = y;
	pic_c = c;
	pic_size_one = pic_x*pic_y;
}

void dvs_processor_cla::dvs_pro_set_frame_period(int time_period)
{
	if (time_period < 1)
	{
		printf("\nError in dvs_pro_set_frame_period, period should >0, now is %d, Need debug\n", time_period);
		time_period = 1000;
	}
	dvs_time_period = time_period;
}

void dvs_processor_cla::dvs_pro_set_event_max(int event_max)
{
	if (event_max < 1)
	{
		printf("\nError in dvs_pro_set_event_max, event_max should >0, now is %d, Need debug\n", event_max);
		event_max = 1000;
	}
	dvs_event_max = event_max;
}

void dvs_processor_cla::dvs_pro_set_fail_times_max(int fail_time_max)
{
	if (fail_time_max < 0)
	{
		printf("\nError in dvs_pro_set_fail_times_max, fail_time_max should >= 0, now is %d, Need debug\n", fail_time_max);
		fail_time_max = 1;
	}
	dvs_cont_fail_max = fail_time_max;

}


void dvs_processor_cla::dvs_pro_set_event_source(int src, char *fname, int len0) // 0:file
{
	data_src = src;
	if (src == 0)
	{
		// copy fname to data_scr_name
		if (len0 >254)
		{
			printf("Error in dvs_pro_set_event_source, file_name length should <255, now is %d, debug\n",len0);
			len0 = 254;
		}
		for(int idx=0; idx<len0; idx++)
		{
			data_scr_name[idx] = fname[idx];
		}
		data_scr_name[len0]= '\0';
		
		init_dvsEventGen(data_scr_name, 0);
	}
	else // do others
	{

	}
	
}

void dvs_processor_cla::dvs_pro_reset_event(void)
{
	dvs_event_cnt 	= 0;
	dvs_time_start 	= 0;	
}

void dvs_processor_cla::dvs_pro_pic_zeros(void)
{
	int cidx,xidx,yidx;
	char *pc, *px, *py;

	pc = dvs_picture;
	for(cidx=0; cidx<pic_c;cidx++)
	{
		px = pc;
		pc += pic_size_one;
		for(xidx=0; xidx<pic_x;xidx++)
		{
			py = px;
			px += pic_y;
			for(yidx=0; yidx<pic_y; yidx++)
			{
				py[yidx] = 0;
			}
		}
	}
}


int dvs_processor_cla::dvs_2_frame_process(int *res_buf)
{
	int time_end, time_cur;
	int cont_fail_cnt;
	int cidx;
	struct str_dvs_event* p_dvs_data = &dvs_data;
	int dvs_get_flag;
	char *p_pic;
	uLint_t pidx;

	// first, init pictures;
	dvs_pro_pic_zeros();
	
	time_end = dvs_time_start + dvs_time_period;
	time_cur = dvs_time_start;

	printf(" Time Start:%d, Time End Needed:%d",dvs_time_start,time_end);
	
	dvs_event_cnt = 0;
	
	p_pic = &dvs_picture[0]; // 0:the first channel of picture, 0*pic_size_one
	
	for(cidx=0,cont_fail_cnt=0; dvs_event_cnt<dvs_event_max; cidx++)
	{
		get_dvs_event(p_dvs_data, &dvs_get_flag);
		if (dvs_get_flag != 0) // get data
		{
			cont_fail_cnt = 0;
			dvs_event_cnt++;
			p_pic[p_dvs_data->x*pic_y + p_dvs_data->y] += p_dvs_data->polarity;
			time_cur = p_dvs_data->time;
			if (time_cur > time_end)
			{
				break;
			}
		}
		else // not get
		{
			cont_fail_cnt++;
			if (cont_fail_cnt > dvs_cont_fail_max)
			{
				time_cur = time_end; // update to latest time. here only example
				break;

			}
			else // try more times
			{

			}
		}
	}
	dvs_time_start = time_cur; // update latest time.

	
	printf(" Time Real End:%d, save event-number is %d\n",dvs_time_start,dvs_event_cnt);

	if (dvs_event_cnt <= 0)
	{
		return FAIL;
	}

	
	// output copy
	for(pidx = 0;pidx<pic_size_one; pidx++)
	{
//		res_buf[pidx] = p_pic[pidx];
		*res_buf++ = *p_pic++;
	}

	return SUCCESS;

}

void test_dvs2frame(void) // an example
{
	dvs_processor_cla * p_test_dvs;
	int size_x,size_y,size_c;
	int time_period;
	int event_max;
	int fail_time_max;
	int data_src_mod;
	
	char *fname = g_dvs_in_name;
	char fout[128];
	int file_cnt;
	int file_len;
	file_len = sizeof(g_dvs_out_name);
	memcpy(fout, g_dvs_out_name,file_len);
	
	int * p_dvs_frame_buf;
	int sidx; // for save
	FILE *fp_save;
	
	printf("In test_dvs2frame, test begin\n");

	size_x = 128;
	size_y = 128;
	size_c = 1;
	time_period = 20000; //2000;
	event_max = 200;
	fail_time_max = 2;
	data_src_mod  = 0; // from file

	p_dvs_frame_buf = (int *)malloc(sizeof(int)*size_x*size_y*size_c);
	if (NULL == p_dvs_frame_buf)
	{
		printf("In test_dvs2frame, Malloc picture is failed. Check it!\n");
		return;
	}

	p_test_dvs = (dvs_processor_cla *)malloc(sizeof(dvs_processor_cla));
	if (NULL == p_test_dvs)
	{
		FREE_POINT(p_dvs_frame_buf);
		printf("In test_dvs2frame, Malloc is failed. Check it!\n");
		return;
	}

	
	p_test_dvs->dvs_processor_init();
	p_test_dvs->dvs_pro_set_pic_size(size_x, size_y, size_c);
	p_test_dvs->dvs_pro_set_frame_period(time_period);
	p_test_dvs->dvs_pro_set_event_max(event_max);
	p_test_dvs->dvs_pro_set_fail_times_max(fail_time_max);
	p_test_dvs->dvs_pro_set_event_source(data_src_mod, fname, strlen(fname)); // 0:file



	file_cnt = 0;
	while(1)
	{
		int *px, *py;

		if (SUCCESS == (p_test_dvs->dvs_2_frame_process(p_dvs_frame_buf)))
		{
			// save to file
			// Process and save
			sprintf(&fout[file_len-1],"_%d",file_cnt);
			file_cnt++;
			
			printf("Save to %s\n",fout);
			fp_save = fopen(fout,"w");
			if (NULL == fp_save)
			{
				printf("In test_dvs2frame, could open file:%s, debug\n",fout);
				
				FREE_POINT(p_test_dvs);
				FREE_POINT(p_dvs_frame_buf);
				return;
			}

			px = p_dvs_frame_buf;
			for(sidx = 0; sidx<size_x; sidx++)
			{	
				py = px;
				px += size_y;
				for(int yidx=0; yidx<size_y; yidx++)
				{
					fprintf(fp_save,"%d ",py[yidx]);
				}
				fprintf(fp_save,"\n");
			}
			fclose(fp_save);

			// for debug
			if (file_cnt >= 200)
			{
				printf("\nIn test_dvs2frame debug, end process\n");
				break;
			}
		}
		else
		{
			break;
		}
	}
	
//	fclose(fp_save);
	
	FREE_POINT(p_test_dvs);
	FREE_POINT(p_dvs_frame_buf);
	
	printf("In test_dvs2frame, test End\n");

}

