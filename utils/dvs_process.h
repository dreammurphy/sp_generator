#ifndef _DVS_PROCESS_H_
#define _DVS_PROCESS_H_

#include "common.h"

#include "dvsEventGen.h"


#define DVS_X_MAX 		(256)
#define DVS_Y_MAX 		(256)
#define DVS_RGB_MAX		(3)

extern void get_dvs_event(struct str_dvs_event* p_dvs_data, int* flag);

extern void test_dvs2frame(void); // an example 


class dvs_processor_cla
{
	public:
	char dvs_picture[DVS_RGB_MAX*DVS_X_MAX*DVS_Y_MAX];

	char data_src;  // 0:file, 1:hardware
	char data_scr_name[256]; // file name
	int  pic_x,pic_y,pic_c; // x,y, channel-RGB
	uLint_t pic_size_one; // one map: pic_x*pic_y;
	int  dvs_event_max; // max number events to generate a frame, 200 etc.
	int  dvs_event_cnt;
	int  dvs_time_start;
	int  dvs_time_period; // using as n us
	int  dvs_cont_fail_max;
	
	struct str_dvs_event dvs_data;

	dvs_processor_cla();
	~dvs_processor_cla();
	
	void dvs_processor_init(void);
	void dvs_pro_set_pic_size(int x, int y, int c=1);
	void dvs_pro_set_frame_period(int time_period);
	void dvs_pro_set_event_max(int event_max);
	void dvs_pro_set_fail_times_max(int fail_time_max);
	void dvs_pro_set_event_source(int src, char *fname, int len0=0); // 0:file

	void dvs_pro_reset_event(void);

	void dvs_pro_pic_zeros(void);
	int dvs_2_frame_process(int *res_buf); // 1:ok, 0:fail

};



#endif
