#ifndef _CALC_UTIL_H_
#define _CALC_UTIL_H_

#include "common.h"

#define LFSR_GROUP_NUM     (32)

class rand_cla
{
	public:
		char rand_len;
		char out_len;
	private:
		char rand_mem[16];
		char out_pos[16];
		char out_bit_buf[16];
		char xor_bit;
		int pos_cur;

	public:
		rand_cla();
		~rand_cla();
		
		void rand_config_pos(int out_len0, char *out_pos0);
		void rand_sta_set(char *mem_16bit, int inv_mod); //inv_mod = 1, mem[15]=mem_16bit[0]
		void rand_sta_set_int(int seed_16bit, int inv_mod); //seed_16bit,  high->bit15, inv_mod:1,high->bit0
		void rand_read_mem_data(int *mem_data, int *out_data); // read data out, out_data is the value for Last output
		void rand_pro(char *rand_out);

};

class Spike_generator
{
	public:
		char spike_in_width;
		int  lut_len;
		char spike_seq_50_256[50][256];

		rand_cla LFSR_gen_buf[LFSR_GROUP_NUM]; //32 LFSR_gen
		rand_cla LFSR_sca_buf[LFSR_GROUP_NUM]; // 32 LFSR_sca
		int group_LFSR;
		int poisson_ratio;

	public:
		Spike_generator();
		~Spike_generator();
		
		int spike_gen_init(void); // 0:OK 1:error
		int spike_LFSR_init(void); // init LFSR_gen and scaler 

		void spike_poisson_set_ration(int ratio); // set poisson_ratio, 8bit, 0 as no effect
		void spike_LFSR_init_set(rand_cla *p_rand_cla, char *seed_16buf, char *out_pos_buf, int out_len, int seed_inv);
		void spike_LFSR_init_set_int(rand_cla *p_rand_cla, int seed_16bit, char *out_pos_buf, int out_len, int seed_inv);
		
		void spike_gen_one(void *in_va_buf,char *out_spike_buf,int tidx, uLint_t spike_len0);
		void spike_gen_lut_one(int *in_va_buf,char *out_spike_buf,int tidx, uLint_t spike_len0);
		void spike_gen_lut_onef(float *in_va_buf,char *out_spike_buf,int tidx, uLint_t spike_len0);
		void spike_gen_poisson_one(int *in_va_buf,char *out_spike_buf,int tidx, uLint_t spike_len0);
		void spike_gen_poisson_onef(float *in_va_buf,char *out_spike_buf,int tidx, uLint_t spike_len0);


};


class neu_unit_para
{
	public:
			
		int init_flg;
		NeuCoreMode neu_mode;  //or MODE-A.B.C.D ??
		int calc_type;			//0:valid, 1:same, 2:full, this time not support full and same
		str_calc_para unit_calc_para;
		int calc_n, calc_group, calc_ki,calc_ktot,calc_wei_tot,calc_x_ah,calc_y_ah; // here, using calc_n=8,  calc_group=4, for calculate 4 results using 32 MAC
		int calc_ix,calc_iy,calc_ox,calc_oy; // not consider the strides
		uLint_t calc_in_size, calc_inner_size, calc_out_size,calc_wei_size;
		uLint_t *p_organize_tbl;
		uLint_t *p_org_wei_tbl;
		uLint_t calc_tbl_len;
		uLint_t calc_time_count;			// counter as time used for this neuron process whole size

		char *p_in_buffer;
		float *p_weight;
		char *p_in_extend_buffer;	// for input extended	
		float *p_inner_buffer;			// for output not considering stride
		float *p_out_buffer;			// for final output
		
	public:
		neu_unit_para();
		~neu_unit_para();
		void neu_release_buf();
		void neu_unit_para_base_init(void);
		void neu_unit_para_int(void);
		void neu_unit_set_para(str_calc_para *p_para,int type, int n, int n_group, NeuCoreMode neu_md);
		void neu_unit_calc_core(void *inX, void *outX);
		void neu_unit_reorder_input(char *inX);
		void neu_unit_reorder_weight(float *p_wei);
} ;



void test_rand(void);
void test_spike_gen(void);
void test_neu_core_calc(void);


int func_val_pos_2int(char * va, int len);


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

void func_neuron_whole_oxy(char *inX, float*p_wei, float *outX, neu_unit_para *p_para);



#endif
