#ifndef _CALC_UTIL_H_
#define _CALC_UTIL_H_

#include "common.h"

class rand_cla
{
	public:
		char rand_len;
		char out_len;
	private:
		char rand_mem[16];
		int out_pos[16];
		char xor_bit;
		int pos_cur;

	public:
		rand_cla();
		~rand_cla();
		
		void rand_config_pos(char out_len0, int *out_pos0);
		void rand_sta_set(char *mem_16bit);

		void rand_pro(char *out_buf);

};

class Spike_generator
{
	public:
		char spike_in_width;
		int  lut_len;
		char spike_seq_50_256[50][256];

	public:
		Spike_generator();
		~Spike_generator();
		
		int spike_gen_init(void); // 0:OK 1:error
		void spike_gen_one(int *in_va_buf,char *out_spike_buf,int tidx, uLint_t spike_len0);
		void spike_gen_one(float *in_va_buf,char *out_spike_buf,int tidx, uLint_t spike_len0);

};


void test_rand(void);
void test_spike_gen(void);


void func_fcn_process();
void func_cnn_process();
void func_pooling_process();

void func_conv2d_spike_pro(char *inX, float *ouX, float *p_wei,int Iy, int Kx, int Ky);
void func_fcn_spike_pro(char *inX, float *ouX, str_calc_para *p_calc_para);
void func_cnn_spike_pro(char *inX, float *ouX, str_calc_para *p_calc_para);
void func_pooling_ave_pro(char *inX, float *ouX, str_calc_para *p_calc_para);
void func_pooling_max_pro(char *inX, float *ouX, str_calc_para *p_calc_para);
void func_concat_pro(char *inX, float *ouX, str_calc_para *p_calc_para);
void func_eltwise_pro(char *inX, float *ouX, str_calc_para *p_calc_para);

void func_find_max(float *in, uLint_t n, uLint_t*midx, float* max_va);


#endif
