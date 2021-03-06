#include "calc_util.h"
#include "stdio.h"
#include "stdlib.h"

int g_Seed_gen_buf[LFSR_GROUP_NUM]	= {\
	0x11,0x31,0x51,0x71,0x91,0xB1,0xD1,0xF1,\
	0x111,0x131,0x151,0x171,0x191,0x1B1,0x1D1,0x1F1,\
	0x211,0x231,0x251,0x271,0x291,0x2B1,0x2D1,0x2F1,\
	0x311,0x331,0x351,0x371,0x391,0x3B1,0x3D1,0x3F1
}; // could configure from files in spike_LFSR_init
int g_Seed_sca_buf[LFSR_GROUP_NUM]	= {
	0x00B,0x02B,0x04B,0x06B,0x08B,0x0AB,0x0CB,0x0EB,\
	0x30B,0x32B,0x34B,0x36B,0x38B,0x3AB,0x3CB,0x3EB,\
	0x60B,0x62B,0x64B,0x66B,0x68B,0x6AB,0x6CB,0x6EB,\
	0x930B,0x932B,0x934B,0x936B,0x938B,0x93AB,0x93CB,0x93EB
}; // could configure from files in spike_LFSR_init


void test_rand_16(int*out_pos, int out_len, char *mem_init, int mem_inv); // mem_inv,1:inv process
void test_rand_16_int(int*out_pos, int out_len, int seed_16, int mem_inv); // mem_inv,1:inv process
void func_gen_poisson_one(int in_va,char *out_spike,rand_cla * p_LFSR_gen,rand_cla * p_LFSR_sca, int ratio);

void func_gen_reorder_tbl(uLint_t *p_tbl, int sx, int sy, int si, int n, int *n_ki);

#if 0
#define print_cnn(format, ...) printf(format, ##__VA_ARGS__)
#else
#define print_cnn(format, ...) {}
#endif


rand_cla::rand_cla()
{
	int idx;
	rand_len = 16;
	for(idx=1; idx<rand_len; idx++)
	{
		rand_mem[idx] = 0;
	}
	rand_mem[0] = 1;

	out_len = 1;
	out_pos[0] = 1;
	
	pos_cur = 0;
	xor_bit = rand_mem[(0+pos_cur)];
}

rand_cla::~rand_cla()
{

}

void rand_cla::rand_sta_set(char *mem_16bit, int inv_mod)
{
	int idx;
	if (inv_mod == 0)
	{
		for(idx=0; idx<rand_len; idx++)
		{
			rand_mem[idx] = mem_16bit[idx];
		}
	}
	else
	{
		for(idx=0; idx<rand_len; idx++)
		{
			rand_mem[idx] = mem_16bit[15-idx];
		}
	}
	pos_cur = 0;
}

void rand_cla::rand_sta_set_int(int seed_16bit, int inv_mod) //seed_16bit,  high->bit15, inv_mod:1,high->bit0
{
	int idx;
	if (inv_mod == 0)
	{
		for(idx=0; idx<16; idx++)
		{
			rand_mem[idx] = (seed_16bit>>idx) & (0x0001);
		}
	}
	else
	{
		for(idx=0; idx<16; idx++)
		{
			rand_mem[15-idx] = (seed_16bit>>idx) & (0x0001);
		}
	}
	pos_cur = 0;
	
}


void rand_cla::rand_config_pos(int out_len0, char *out_pos0)
{
	int idx;
	for(idx=0; idx<out_len0; idx++)
	{
		out_pos[idx] = out_pos0[idx];
	}
	out_len = out_len0;
	pos_cur = 0;
} 

void rand_cla::rand_pro(char *rand_out)
{
	char bit16,bit14,bit13,bit11;
	int idx;
	xor_bit = rand_mem[pos_cur];
	bit11 = rand_mem[(pos_cur+11)&0x0f] ^ xor_bit;
	bit13 = rand_mem[(pos_cur+13)&0x0f] ^ xor_bit;
	bit14 = rand_mem[(pos_cur+14)&0x0f] ^ xor_bit;
	bit16 = xor_bit;
	for(idx=0; idx<out_len; idx++)
	{
		out_bit_buf[idx] = rand_mem[(pos_cur + out_pos[idx])&0x0f];
		rand_out[idx] = out_bit_buf[idx];
	}
	
	pos_cur++;
	rand_mem[(pos_cur+10)&0x0f] = bit11;
	rand_mem[(pos_cur+12)&0x0f] = bit13;
	rand_mem[(pos_cur+13)&0x0f] = bit14;
	rand_mem[(pos_cur+15)&0x0f] = bit16;
	
	//if(pos_cur >= rand_len) // rand_len = 16
	//	pos_cur -= rand_len;
	pos_cur = pos_cur & 0x0f;

}

void rand_cla::rand_read_mem_data(int *mem_data, int *out_data) //out_data is the value for Last output
{
	char mem_buf[16];
	int idx;
	for(idx=0; idx<16; idx++)
	{
		mem_buf[idx] = rand_mem[(pos_cur+idx)&0x0f];
	}

	*mem_data = func_val_pos_2int(mem_buf, 16);
	*out_data = func_val_pos_2int(out_bit_buf, out_len); // using old output value for last out

}


int func_val_pos_2int(char * va, int len)
{
	int sum0 = 0;
	int idx;
	if ((len <0) || (len > 16)) // 0 <= len <= 16
	{
		printf("Error In func_val_pos_2int, len should in [0,16], now is %d, debug\n",len);
		return 0;
	}

	for(idx=0; idx<len; idx++)
	{
		sum0 += (((int)va[idx])<<idx);
	}
	return sum0;
}

void test_rand_16(char*out_pos, int out_len, char *p_mem_in, int mem_inv)
{
	rand_cla rand_1;
	char rand_out[16];
	int mem_data,pos_data;

	int idx;

	rand_1.rand_config_pos(out_len,out_pos);
	rand_1.rand_sta_set(p_mem_in,mem_inv);

	for(idx=0; idx<16; idx++)
	{
		rand_1.rand_pro(rand_out);
		rand_1.rand_read_mem_data(&mem_data, &pos_data);
		// output, print
		printf("Idx %d,output:%d,%d,%d, As %0X, Inner:%0X, using Decimal:%d,%d\n",idx,rand_out[0],rand_out[1],rand_out[2],\
		func_val_pos_2int(rand_out, out_len),mem_data,func_val_pos_2int(rand_out, out_len),mem_data);
	}

}

void test_rand_16_int(char*out_pos, int out_len, int seed_16, int mem_inv)
{
	rand_cla rand_1;
	char rand_out[16];
	int mem_data,pos_data;

	int idx;

	rand_1.rand_config_pos(out_len,out_pos);
	rand_1.rand_sta_set_int(seed_16,mem_inv);

	for(idx=0; idx<16; idx++)
	{
		rand_1.rand_pro(rand_out);
		rand_1.rand_read_mem_data(&mem_data, &pos_data);
		// output, print
		printf("Idx %d,output:%d,%d,%d, As %0X, Inner:%0X, using Decimal:%d,%d\n",idx,rand_out[0],rand_out[1],rand_out[2],\
		func_val_pos_2int(rand_out, out_len),mem_data,func_val_pos_2int(rand_out, out_len),mem_data);
	}

}


void test_rand(void)
{
	char out_pos[5] = {10,11,12,13,14};
	int out_len = 5;
//	char mem_init[16] = {1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1};
	char mem_init_inv[16] = {1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1};

	int seed_16 = 0x0B6A9;
	
	test_rand_16(out_pos, out_len, mem_init_inv, 1);
	printf("rand test-1 end\n");
	
	test_rand_16_int(out_pos, out_len, seed_16, 0);
	printf("rand test-1 seed input end\n");


	char out_pos2[8] = {8,9,10,11,12,13,14,15};
	out_len = 8;
	
//	char mem_init2[16] = {0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0};
	char mem_init_inv2[16] = {0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0};
	seed_16 = 0x725E;

	test_rand_16(out_pos2, out_len, mem_init_inv2, 1);

	printf("rand test-2 end\n");
	
	test_rand_16_int(out_pos2, out_len, seed_16, 0);

	printf("rand test-2 seed input end\n");

}

Spike_generator::Spike_generator()
{
	group_LFSR 		= LFSR_GROUP_NUM;
	lut_len			= 50;
	poisson_ratio 	= 0; // ignore ratio
}

Spike_generator::~Spike_generator()
{

}

int Spike_generator::spike_gen_init(void)
{
	FILE *fp;
	char file_str[] = "./data/spike_seq_50_256.txt";
	int va;
	fp = fopen(file_str,"r");
	if(NULL == fp)
	{
		printf("File %s Could not Open, need debug\n",file_str);
		return 1; // Error
	}
	
	spike_in_width = 8;
	lut_len = 50;
	
	for(int idx=0; idx<lut_len; idx++)
	{
		for(int sidx=0; sidx<256; sidx++)
		{
			fscanf(fp,"%d",&va);
			spike_seq_50_256[idx][sidx] = va;
		}
	}
	fclose(fp);

	spike_LFSR_init(); // for Possion LFSR
	
	return 0;
}

void Spike_generator::spike_poisson_set_ration(int ratio) // set poisson_ratio, 8bit, 0 as no effect
{
	if ((ratio <0 ) || (ratio > 127))
	{
		printf("Error in spike_poisson_set_ration, value should be [0,127], now is %d, debug\n",ratio);
		ratio = 0;
	}
	poisson_ratio = ratio;
}

int Spike_generator::spike_LFSR_init(void)
{
	// first, LFSR seed init
//	int seed_gen_buf[LFSR_GROUP_NUM] 	= {};
//	int seed_sca_buf[LFSR_GROUP_NUM] 	= {};

	// output position 
	char out_pos_gen_buf[5] = {10,11,12,13,14};
	char out_pos_sca_buf[8] = {8,9,10,11,12,13,14,15};
	int out_gen_len = 5;
	int out_sca_len = 8;
	int ratio		 = 0; // poisson ratio

	int idx;
	
	group_LFSR = LFSR_GROUP_NUM;

	spike_poisson_set_ration(ratio);
	
	for(idx=0; idx<LFSR_GROUP_NUM; idx++)
	{
		spike_LFSR_init_set_int(&LFSR_gen_buf[idx], g_Seed_gen_buf[idx], out_pos_gen_buf, out_gen_len, 0);
		spike_LFSR_init_set_int(&LFSR_sca_buf[idx], g_Seed_sca_buf[idx], out_pos_sca_buf, out_sca_len, 0);

	}

	return 0;
}



void Spike_generator::spike_LFSR_init_set(rand_cla *p_rand_cla, char *seed_16buf, char *out_pos_buf, int out_len, int seed_inv)
{
	if ((out_len <0) || (out_len > 16))
	{
		printf("Error in spike_LFSR_init_set, out_len should be in [0,16], now is %d, Debug\n",out_len);
		exit(0);
	}

	p_rand_cla->rand_config_pos(out_len,out_pos_buf);
	p_rand_cla->rand_sta_set(seed_16buf,seed_inv);

}

void Spike_generator::spike_LFSR_init_set_int(rand_cla *p_rand_cla, int seed_16bit, char *out_pos_buf, int out_len, int seed_inv)
{
	if ((out_len <0) || (out_len > 16))
	{
		printf("Error in spike_LFSR_init_set, out_len should be in [0,16], now is %d, Debug\n",out_len);
		exit(0);
	}

	p_rand_cla->rand_config_pos(out_len,out_pos_buf);
	p_rand_cla->rand_sta_set_int(seed_16bit,seed_inv);

}

void Spike_generator::spike_gen_one(void *in_va_buf,char *out_spike_buf,int tidx, uLint_t spike_len0)
{
	int *p_in = (int *)in_va_buf;
	float *p_if = (float *)in_va_buf;
	switch (SPIKE_GENE_SEL)
	{
		case 0:
		case 1:
			spike_gen_lut_one(p_in,out_spike_buf,tidx, spike_len0);
			break;
			
		case 2:
			spike_gen_lut_onef(p_if,out_spike_buf,tidx, spike_len0);
			break;	
			
		case 3:
			spike_gen_poisson_one(p_in,out_spike_buf,tidx, spike_len0);
			break;	
			
		case 4:
			spike_gen_poisson_onef(p_if,out_spike_buf,tidx, spike_len0);
			break;	
			
		default:
			spike_gen_lut_onef(p_if,out_spike_buf,tidx, spike_len0);
			break;
	}

}

void Spike_generator::spike_gen_lut_one(int *in_va_buf,char *out_spike_buf,int tidx, uLint_t spike_len0)
{
	uLint_t idx;
	char *p_in;
	p_in = spike_seq_50_256[tidx];
	for(idx =0; idx<spike_len0; idx++)
	{
		out_spike_buf[idx] = p_in[in_va_buf[idx]];
	}
}


void Spike_generator::spike_gen_poisson_one(int *in_va_buf,char *out_spike_buf,int tidx, uLint_t spike_len0)
{
	uLint_t idx;
	int group_idx;
	int in_va;
	
	for(idx =0,group_idx=0; idx<spike_len0; idx++)
	{
		in_va = in_va_buf[idx];
		func_gen_poisson_one(in_va,&out_spike_buf[idx],&LFSR_gen_buf[group_idx],&LFSR_sca_buf[group_idx], poisson_ratio);

		group_idx++;
		if(group_idx >= group_LFSR)
			group_idx = 0;
	}

}


void Spike_generator::spike_gen_lut_onef(float *in_va_buf,char *out_spike_buf,int tidx, uLint_t spike_len0)
{
	uLint_t idx;
	char *p_in;
	int in_idx;
	p_in = spike_seq_50_256[tidx];
	for(idx =0; idx<spike_len0; idx++)
	{
		in_idx = int(in_va_buf[idx]*256);
		if(in_idx>255)
			in_idx = 255;

		out_spike_buf[idx] = p_in[in_idx];
	}
}

void Spike_generator::spike_gen_poisson_onef(float *in_va_buf,char *out_spike_buf,int tidx, uLint_t spike_len0)
{
	uLint_t idx;
	int group_idx;
	int in_va;
	
	for(idx =0,group_idx=0; idx<spike_len0; idx++)
	{
		in_va = int(in_va_buf[idx]*256);
		func_gen_poisson_one(in_va,&out_spike_buf[idx],&LFSR_gen_buf[group_idx],&LFSR_sca_buf[group_idx], poisson_ratio);

		group_idx++;
		if(group_idx >= group_LFSR)
			group_idx = 0;
	}

}

void test_spike_gen(void)
{
	Spike_generator test_spike_gen;
	int slen = 50;
	int in_va_buf[8] = {10,20,40,60,80,120,160,230};
	char ou_spike_buf[8];
	if (0 != test_spike_gen.spike_gen_init())
	{
		printf("Error in test_spike_gen, need debug!!! \n");
		return;
	}
	for(int idx=0; idx<10; idx++)
	{
		printf("Spike-Generator, time=%d,[in*256,out] ",idx);
		test_spike_gen.spike_gen_one(in_va_buf,ou_spike_buf,idx, slen);
		for(int sidx=0; sidx<slen; sidx++)
		{
			printf(",[%d,%d]",in_va_buf[sidx],ou_spike_buf[sidx]);
		}
		printf("\n");
	}
	printf("Spike-Generator Now using Poisson \n");

	int poi_ratio = 13;  // 10%*128=13
	test_spike_gen.spike_poisson_set_ration(poi_ratio);
	for(int idx=0; idx<20; idx++)
	{
		printf("Spike-Generator, time=%d,[in*256,out] ",idx);
		test_spike_gen.spike_gen_poisson_one(in_va_buf,ou_spike_buf,idx, slen);
		for(int sidx=0; sidx<slen; sidx++)
		{
			printf(",[%d,%d]",in_va_buf[sidx],ou_spike_buf[sidx]);
		}
		printf("\n");
	}	
	printf("Spike-Generator Poisson test End\n");
}

void func_gen_poisson_one(int in_va,char *out_spike,rand_cla * p_LFSR_gen,rand_cla * p_LFSR_sca, int ratio)
{
	char rand_gen[16]; // 5bit
	char rand_sca[16]; // 8bit
	int rand_gen_data;
	int rand_sca_data;
	p_LFSR_gen->rand_pro(rand_gen);
	rand_gen_data = func_val_pos_2int(rand_gen, p_LFSR_gen->out_len);
	p_LFSR_sca->rand_pro(rand_sca);
	rand_sca_data = func_val_pos_2int(rand_sca, p_LFSR_sca->out_len);

	if ((rand_gen_data<= in_va) && (ratio <= rand_sca_data))
	{
		*out_spike = 1;
	}
	else
	{
		*out_spike = 0;
	}
}


void func_fcn_process(void)
{
}
void func_cnn_process(void)
{
}
void func_pooling_process(void)
{

}

void func_conv2d_spike_pro(char *inX, float *ouX, float *p_wei,int Iy, int Kx, int Ky)
{
//	int coidx, cidx;
	int kxidx, kyidx;
	char *p_ix, *p_iy;
	float *p_kx, *p_ky;
	float tmp_sum0;
	tmp_sum0 = 0;
	p_kx = p_wei;
	p_ix = inX;
	for(kxidx=0; kxidx < Kx; kxidx++)
	{
		p_ky = p_kx;
		p_kx += Ky;
		p_iy = p_ix;
		p_ix += Iy;

		for(kyidx=0; kyidx < Ky; kyidx++)
		{
//			tmp_sum0 += p_iy[kyidx]*p_ky[kyidx];
			if (p_iy[kyidx] != 0)
				tmp_sum0 += p_ky[kyidx];
		}
	}
	*ouX = tmp_sum0;

}

int g_deb_print = 0;
#define DEB_PRINT_NUM	(30)

//FILE * fp_deb_spike_fcn;
FILE * fp_deb_spike_cnn;
void func_fcn_spike_pro(char *inX, float *ouX, str_calc_para *p_calc_para)
{
	int coidx, cidx;
	float *p_wei, *p_ko;
	float *p_bias;

	if ((p_calc_para->Kx != 1) || (p_calc_para->Ky != 1) || (p_calc_para->Ix != 1) \
		|| (p_calc_para->Iy != 1))
	{
		printf("Error in func_fcn_spike_pro,parametes not correct. Need debug! \n");
		return;
	}

	p_wei = p_calc_para->p_weight;

#if (DEB_PRINT_NUM > 0)
	// for debug more
	float *p_tmp_o;
	p_tmp_o = (float *)malloc(p_calc_para->Co*sizeof(float));
	if (NULL == p_tmp_o)
	{
		printf("Error in func_fcn_spike_pro, memory is not enough, debug\n");
		exit(1);
		return;
	}
	for(coidx=0;coidx<p_calc_para->Co; coidx++)
	{
		p_tmp_o[coidx] = 0;
	}
	for(cidx=0; cidx< p_calc_para->Ci; cidx++)
	{
		p_ko = p_wei;
		p_wei++;
		
		if (inX[cidx]!= 0)
		{
			for(coidx=0; coidx<p_calc_para->Co; coidx++)
			{
				p_tmp_o[coidx] += p_ko[0];
				p_ko += p_calc_para->Ci; // Next Ker point
			}
		}
	}
	
	if(p_calc_para->bias_en != 0)
	{	
		p_bias = p_calc_para->p_bias;
		for(coidx=0; coidx<p_calc_para->Co; coidx++)
		{
			p_tmp_o[coidx]+= p_bias[coidx];
		}
	}
	for(coidx=0;coidx<p_calc_para->Co; coidx++)
	{
		ouX[coidx] +=p_tmp_o[coidx];
	}
	

	// for debug
	if (g_deb_print < DEB_PRINT_NUM)
	{
		p_wei = p_calc_para->p_weight;

		#if (1 == CASE_TEST)
		printf("DEBUG DATA, input,[Ci,Co,inX,wei]:\n");
		for(cidx=0; cidx< p_calc_para->Ci; cidx++)
		{
			p_ko = p_wei;
			p_wei++;

				for(coidx=0; coidx<p_calc_para->Co; coidx++)
				{
					printf("%d, %d, %d, %f\n",cidx,coidx,inX[cidx],p_ko[0]);
					fprintf(fp_deb_spike_cnn,"%d, %d, %d, %f\n",cidx,coidx,inX[cidx],p_ko[0]);
					p_ko += p_calc_para->Ci; // Next Ker point
				}
		}
		
		printf("DEBUG DATA, output:\n");
			for(coidx=0; coidx<p_calc_para->Co; coidx++)
			{
				printf("%d, %f, %f\n",coidx,ouX[coidx],p_tmp_o[coidx]);
				fprintf(fp_deb_spike_cnn,"-1, %d, %f, %f\n",coidx,ouX[coidx],p_tmp_o[coidx]);
			}
		#elif (2 == CASE_TEST)
		
		for(cidx=0; cidx< p_calc_para->Ci; cidx++)
		{
			fprintf(fp_deb_spike_cnn,"-3, -1, %d, %d, %d\n",cidx,coidx,inX[cidx]);
		}
		
		printf("DEBUG DATA, input,[-3,Ci,Co,inX,wei]:\n");

//		for(cidx=0; cidx< p_calc_para->Ci; cidx++)
		for(coidx=0; coidx<p_calc_para->Co; coidx++)
		{
			p_ko = p_wei;
			p_wei+= p_calc_para->Ci;
				for(cidx=0; cidx< p_calc_para->Ci; cidx++)
				{
					printf("-3, %d, %d, %d, %f\n",cidx,coidx,inX[cidx],p_ko[cidx]);
					fprintf(fp_deb_spike_cnn,"-3, %d, %d, %d, %f\n",cidx,coidx,inX[cidx],p_ko[cidx]);
				}
		}
		
		printf("DEBUG DATA, output:\n");
			for(coidx=0; coidx<p_calc_para->Co; coidx++)
			{
				printf("-3, -2, %d, %f, %f\n",coidx,ouX[coidx],p_tmp_o[coidx]);
				fprintf(fp_deb_spike_cnn,"-3, -2, %d, %f, %f\n",coidx,ouX[coidx],p_tmp_o[coidx]);
			}
			#endif

			
			g_deb_print++;
			
			if(g_deb_print == DEB_PRINT_NUM)
				fclose(fp_deb_spike_cnn);
	}

	FREE_POINT(p_tmp_o);

	// End debug	

#else

	for(cidx=0; cidx< p_calc_para->Ci; cidx++)
	{
		p_ko = p_wei;
		p_wei++;
		
		if (inX[cidx]!= 0)
		{
			for(coidx=0; coidx<p_calc_para->Co; coidx++)
			{
				ouX[coidx] += p_ko[0];
				p_ko += p_calc_para->Ci; // Next Ker point
			}
		}
	}
	
	if(p_calc_para->bias_en != 0)
	{	
		p_bias = p_calc_para->p_bias;
		for(coidx=0; coidx<p_calc_para->Co; coidx++)
		{
			ouX[coidx]+= p_bias[coidx];
		}
	}
#endif

}

// Note, in func_cnn_spike_pro, ouX should have been initialized
void func_cnn_spike_pro(char *inX, float *ouX, str_calc_para *p_calc_para)
{
// in:Y*X*Ci, Ou:Y*X*Co, weight:Ky*Kx*Ci*Co
	int oyidx,oxidx,ocidx,icidx;
	uLint_t map_size_o,map_size_i, map_ker, map_ker_a;
	float *p_co, *p_xo, *p_yo;
	char *p_ci, *p_xi, *p_yi, *p_xyi;
	float *p_ko, *p_ki, *p_kx; //, *p_ky;

	float tmp_sum0;

	map_size_o = p_calc_para->Ox*p_calc_para->Oy;
	map_size_i = p_calc_para->Ix*p_calc_para->Iy;
	map_ker    = p_calc_para->Kx*p_calc_para->Ky;
	map_ker_a  = map_ker*p_calc_para->Ci;
	p_co = ouX;
	p_ko  = p_calc_para->p_weight;

	#if 0		// for debug
	printf("Parameters:in:[x,y,ci]:[%d,%d,%d],ker:[x,y,co]:[%d,%d,%d]\n",p_calc_para->Ix,p_calc_para->Iy,p_calc_para->Ci,
		p_calc_para->Kx,p_calc_para->Ky,p_calc_para->Co);
	printf("Parameters:out:[x,y]:[%d,%d],size_out:%lu\n",p_calc_para->Ox,p_calc_para->Oy,p_calc_para->size_out);
	#endif
	
	#if (DEB_PRINT_NUM > 0)
		// for debug more
		float *p_tmp_o;
		p_tmp_o = (float *)malloc(p_calc_para->size_out*sizeof(float));
		if (NULL == p_tmp_o)
		{
			printf("Error in func_cnn_spike_pro, memory is not enough, debug\n");
			exit(1);
			return;
		}
		
		for(uLint_t i0 =0; i0 < p_calc_para->size_out; i0++)
		{
			p_tmp_o[i0]=0;
		}

		printf("debug func_cnn_spike_pro - 1\n");
		
		p_co = p_tmp_o;
		for(ocidx=0; ocidx < p_calc_para->Co; ocidx++)
		{
			p_xo = p_co; 
			p_co += map_size_o;
			
			p_xi = inX;
			for(oxidx=0; oxidx < p_calc_para->Ox; oxidx++)
			{
				p_yo = p_xo;
				p_xo += p_calc_para->Oy;

				p_yi = p_xi;
				p_xi += p_calc_para->Iy;	// *stride_x, stride_x= 1			
				for(oyidx=0; oyidx < p_calc_para->Oy; oyidx++)
				{
					if ((0) && (oyidx == 5)) // for debug
					{
						char *p_debx;
						p_debx = &p_yi[3*p_calc_para->Iy];
						printf("%d,%d,%d,%d,%d\n",p_debx[0],p_debx[1],p_debx[2],p_debx[3],p_debx[4]);
						p_debx = &p_yi[4*p_calc_para->Iy];
						printf("%d,%d,%d,%d,%d\n",p_debx[0],p_debx[1],p_debx[2],p_debx[3],p_debx[4]);
					}
					
					// each kernel
					tmp_sum0 = p_yo[oyidx];
					p_xyi = p_yi;
					p_yi++; // *stride_y, stride_y = 1
					
					p_ki = p_ko; // init Kernel
					for(icidx = 0; icidx<p_calc_para->Ci; icidx++)
					{
						float sum0;
						p_ci = p_xyi;
						p_xyi += map_size_i;
						p_kx = p_ki;
						p_ki += map_ker;
						func_conv2d_spike_pro(p_ci, &sum0, p_kx, p_calc_para->Iy, p_calc_para->Kx, p_calc_para->Ky);
						
						tmp_sum0 += sum0;
					} // end icidx
					p_yo[oyidx] = tmp_sum0;
				} // end oyidx
		
			}// end oxidx
			
			p_ko += map_ker_a;
			
		}// end ocidx

		
		printf("debug func_cnn_spike_pro - 3\n");

		for(uLint_t i0=0;i0<p_calc_para->size_out; i0++)
		{
			ouX[i0] +=p_tmp_o[i0];
		}
	
		// for debug
		if (g_deb_print < DEB_PRINT_NUM)
		{
			int cidx,coidx,xidx,yidx;
			int size_in0;
			size_in0 = p_calc_para->Ix*p_calc_para->Iy;
			printf("DEBUG DATA,CNN input,[Ci,xi,yi,in]:\n");
			for(cidx=0; cidx<p_calc_para->Ci; cidx++)
			{
				for(xidx=0; xidx<p_calc_para->Ix;xidx++)
				{
					for(yidx=0; yidx<p_calc_para->Iy;yidx++)
					{
						if (cidx==0)
						{
							printf("-1, %d, %d, %d, %d\n",cidx,xidx,yidx,inX[cidx*size_in0+xidx*p_calc_para->Iy+yidx]);
						}
						fprintf(fp_deb_spike_cnn,"-1, %d, %d, %d, %d\n",cidx,xidx,yidx,inX[cidx*size_in0+xidx*p_calc_para->Iy+yidx]);
					}
				}
			}
			printf("DEBUG DATA,CNN weight,[Co,ci,kx,ky,wei]:\n");
			p_ko = p_calc_para->p_weight;
			for(coidx=0; coidx< p_calc_para->Co; coidx++)
			{
				p_ki = p_ko;
				p_ko += map_ker_a;
	
				for(cidx=0; cidx<p_calc_para->Ci; cidx++)
				{
					p_kx = p_ki;
					p_ki += map_ker;
					for(xidx=0; xidx<p_calc_para->Kx;xidx++)
					{
						for(yidx=0; yidx<p_calc_para->Ky;yidx++)
						{
							if ((coidx==1) && (0 == cidx))
							{
								printf("%d, %d, %d, %d, %f\n",coidx,cidx,xidx,yidx,p_ki[xidx*p_calc_para->Ky+yidx]);
							}
							
							fprintf(fp_deb_spike_cnn,"%d, %d, %d, %d, %f\n",coidx,cidx,xidx,yidx,p_kx[xidx*p_calc_para->Ky+yidx]);
						}
					}
				}

			}
			
			printf("DEBUG DATA, CNN output:\n");
			size_in0 = p_calc_para->Ox*p_calc_para->Oy;
			for(cidx=0; cidx<p_calc_para->Co; cidx++)
			{
				for(xidx=0; xidx<p_calc_para->Ox;xidx++)
				{
					for(yidx=0; yidx<p_calc_para->Oy;yidx++)
					{
						if (cidx==0)
						{
							printf("-2, %d, %d, %d, %f\n",cidx,xidx,yidx,p_tmp_o[cidx*size_in0+xidx*p_calc_para->Oy+yidx]);
						}
						fprintf(fp_deb_spike_cnn,"-2, %d, %d, %d, %f\n",cidx,xidx,yidx,p_tmp_o[cidx*size_in0+xidx*p_calc_para->Oy+yidx]);
					}
				}
			}

				g_deb_print++;
				
				if(g_deb_print == DEB_PRINT_NUM)
					fclose(fp_deb_spike_cnn);
		}
	
		FREE_POINT(p_tmp_o);
	
		// End debug	

	#else	
	for(ocidx=0; ocidx < p_calc_para->Co; ocidx++)
	{
		p_xo = p_co; 
		p_co += map_size_o;
		p_ki = p_ko;
		p_ko += map_ker_a;
		
		p_xi = inX;
		for(oxidx=0; oxidx < p_calc_para->Ox; oxidx++)
		{
			p_yo = p_xo;
			p_xo += p_calc_para->Oy;
		
			p_yi = p_xi;
			p_xi += p_calc_para->Iy;	// *stride_x, stride_x= 1			
			for(oyidx=0; oyidx < p_calc_para->Oy; oyidx++)
			{
				// each kernel
				tmp_sum0 = p_yo[oyidx];
				p_xyi = p_yi;
				p_yi++; // *stride_y, stride_y = 1
				for(icidx = 0; icidx<p_calc_para->Ci; icidx++)
				{
					float sum0;
					p_ci = p_xyi;
					p_xyi += map_size_i;
					p_kx = p_ki;
					p_ki += map_ker;
					func_conv2d_spike_pro(p_xi, &sum0, p_kx, p_calc_para->Iy, p_calc_para->Kx, p_calc_para->Ky);
					
					tmp_sum0 += sum0;
				} // end icidx
				p_yo[oyidx] = tmp_sum0;
			} // end oyidx

		}// end oxidx
	}// end ocidx
	#endif
}

void func_pooling_ave_pro(char *inX, float *ouX, str_calc_para *p_calc_para)
{
// in:Y*X*Ci, Ou:Y*X*Co, weight:Ky*Kx*Ci*Co
	int oyidx,oxidx,ocidx,ixidx,iyidx;
	uLint_t map_size_o,map_size_i;
	float *p_co, *p_xo, *p_yo;
	char *p_ci, *p_xi, *p_yi;
	char *p_cur;
//	float *p_ko, *p_ki, *p_kx; //, *p_ky;

	float tmp_sum0;

	map_size_o = p_calc_para->Ox*p_calc_para->Oy;
	map_size_i = p_calc_para->Ix*p_calc_para->Iy;
	p_co = ouX;
	p_ci = inX;
//	p_ko  = p_calc_para->p_weight;

	// for pooling -average
	for(ocidx=0; ocidx < p_calc_para->Co; ocidx++) // each co,ox,oy, do sum-average
	{
		p_xo = p_co; 
		p_co += map_size_o;
		p_xi = p_ci;
		p_ci += map_size_i;
//		p_ki = p_ko;
//		p_ko += map_ker;
		for(oxidx=0; oxidx < p_calc_para->Ox; oxidx++)
		{
			p_yo = p_xo;
			p_xo += p_calc_para->Oy;
			p_yi = p_xi;
			p_xi += p_calc_para->Iy * p_calc_para->stride_x;
			for(oyidx=0; oyidx < p_calc_para->Oy; oyidx++)
			{
				// each kernel
				tmp_sum0 = 0; // then, add inx[ix,iy], range:kx,ky, and next point, str_x,str_y
				p_cur = p_yi;
				p_yi += p_calc_para->stride_y;
				for(ixidx = 0; ixidx<p_calc_para->Kx; ixidx++)
				{
					for(iyidx = 0; iyidx<p_calc_para->Ky; iyidx++)
					{				
						tmp_sum0 += p_cur[iyidx];
					}
					p_cur += p_calc_para->Iy;
				} // end ixidx
				
				p_yo[oyidx] = tmp_sum0;
			} // end oyidx

		}// end oxidx
	}// end ocidx
}


void func_pooling_max_pro(char *inX, float *ouX, str_calc_para *p_calc_para)
{

}
void func_concat_pro(char *inX, float *ouX, str_calc_para *p_calc_para)
{

}
void func_eltwise_pro(char *inX, float *ouX, str_calc_para *p_calc_para)
{

}


void func_find_max(float *in, uLint_t n, uLint_t *midx, float* max_va)
{
	uLint_t idx;
	uint_t max_idx;
	float va0;
	va0 = in[0];
	max_idx = 0;
	for(idx=1; idx<n; idx++)
	{
		if (va0 < in[idx])
		{
			va0 = in[idx];
			max_idx = idx;
		}
	}
	*midx = max_idx;
	*max_va = va0;
	
}

// inX:[ci][x][y], p_wei:one weight buffer after re-organize, while inX should also be organized, 
// here, get n_group results, each have n coef, total inX: n*n_group, while, p_wei only n elements
// for this, n=8, n_group=4, 
// note: inX[n_group][n], p_wei[n]; outX shold be set to 0 if it's the first time to calculate
void func_neuron_one_unit(char *inX, float*p_wei, float *outX, int n, int n_group)
{
	int gidx, nidx;
	float sum0;
	char * p_in;
	
	p_in = inX;
	for(gidx=0; gidx<n_group; gidx++)
	{
		sum0 = outX[gidx];
		for(nidx=0; nidx<n; nidx++)
		{
			print_cnn("func_neuron_one_unit,[group,n,sum0-before]:[%d,%d,%f]\n",n_group,n,sum0);
			if (p_in[nidx] != 0)
				sum0 += p_wei[nidx];

			print_cnn("func_neuron_one_unit,[gidx,nidx,in,weight,sum0]:[%d,%d,%d,%f,%f]\n",gidx,nidx,p_in[nidx],p_wei[nidx],sum0);
		}
		outX[gidx] = sum0;
		p_in += n;
	}
}

// func_neuron_one_kx, same as func_neuron_one_unit; kernel not change, while support more n_group
// if more detail, n_group should be sliced every 4 groups as in func_neuron_one_unit
// stride = 1, not take the stride into accout,
void func_neuron_one_kx(char *inX, float*p_wei, float *outX, int n, int n_group, int group_base)
{
//	ASSERT2(n_group,group_base,"func_neuron_whole_kx");

	// n_group should >= group_base, and then split to group_base, here not using this
	if (n_group >group_base)
	{
		func_neuron_one_unit(inX, p_wei, outX, n, n_group);
	}
	else
	{
		func_neuron_one_unit(inX, p_wei, outX, n, n_group); // group<base,pad to zeros
	}
	
}

// func_neuron_whole_kx, fix channel_in direction, and loop the kx, while support more n_group
// stride = 1, not take the stride into accout,
// note, kernel weight buffer have been re-organized
void func_neuron_whole_kx(char *inX, float*p_wei, float *outX, int n, int ix, int kx, int group_base)
{
	int kxidx,kn;
	float *p_wei_in;
	char *p_in;
	float *p_ou;
	
	p_in = inX;
	p_ou = outX;
	p_wei_in = p_wei;
	
	ASSERT2(ix,kx,"func_neuron_whole_kx");

	kn=ix-kx+1;
	for (kxidx =0; kxidx < kx; kxidx++)
	{
		// for debug
		print_cnn("func_neuron_whole_kx,kxidx:%d,toal:[ix,wei]=[%d,%d], X:\n",kxidx,ix,kx);
		for(int i0= 0;i0<n*(ix - kxidx);i0++)
		{
			print_cnn("%d,",p_in[i0]);
		}
		print_cnn("\nfunc_neuron_whole_kx,Weight:\n");
		for(int i0=0;i0<kx; i0++)
		{
			print_cnn("%f,",p_wei_in[i0]);
		}
		print_cnn("\n");
		// end debug print
			
		func_neuron_one_kx(p_in, p_wei_in, p_ou, n,kn,group_base);
		p_in += n;
		p_wei_in += n;
		
	}
	
}

// func_neuron_whole_kxi, fix channel_in direction, and loop the kx, the loop the ki 
// stride = 1, not take the stride into accout,
// note, kernel weight buffer have been re-organized,i.e., weight[ki][kx][n], inX[ki][x][n]
void func_neuron_whole_kxi(char *inX, float*p_wei, float *outX, int n, int ix, int kx, int ki, int group_base)
{
	int kidx;
	float *p_wei_in;
	char *p_in;
	float *p_ou;
	int xmap,wmap;
	
	p_in = inX;
	p_ou = outX;
	p_wei_in = p_wei;
	xmap = n*ix;
	wmap = n*kx;
	
	ASSERT2(ix,kx,"func_neuron_whole_kxi");

	for (kidx =0; kidx < ki; kidx++)
	{
		// for debug
		print_cnn("func_neuron_whole_kxi,kxidx:%d, X:\n",kidx);
		for(int i0=0;i0<xmap;i0++)
		{
			print_cnn("%d,",p_in[i0]);
		}
		print_cnn("\nfunc_neuron_whole_kxi,Weight:\n");
		for(int i0=0;i0<wmap; i0++)
		{
			print_cnn("%f,",p_wei_in[i0]);
		}
		print_cnn("\n");
		// end debug print
		
		func_neuron_whole_kx(p_in, p_wei_in, p_ou, n, ix,kx,group_base);
		p_in += xmap;
		p_wei_in += wmap;
	}
	
}


// func_neuron_whole_kxiy, fix channel_in direction, and loop the kx, the loop the ki , finally the ky
// stride = 1, not take the stride into accout,
// note, kernel weight buffer have been re-organized,i.e., weight[ky][ki][kx][n], inX[ky][ki][x][n]
void func_neuron_whole_kxiy(char *inX, float*p_wei, float *outX, int n, int ix, int kx, int ki, int ky, int group_base)
{
	int kidx;
	float *p_wei_in;
	char *p_in;
	float *p_ou;
	int xmap,wmap;
	
	p_in = inX;
	p_ou = outX;
	p_wei_in = p_wei;
	xmap = n*ix*ki;
	wmap = n*kx*ki;
	
	ASSERT2(ix, kx,"func_neuron_whole_kxiy");

	for (kidx =0; kidx < ky; kidx++)
	{
		// for debug
		print_cnn("func_neuron_whole_kxiy,kxidx:%d, X:\n",kidx);
		for(int i0=0;i0<xmap;i0++)
		{
			print_cnn("%d,",p_in[i0]);
		}
		print_cnn("\nfunc_neuron_whole_kxiy,Weight:\n");
		for(int i0=0;i0<wmap; i0++)
		{
			print_cnn("%f,",p_wei_in[i0]);
		}
		print_cnn("\n");
		// end debug print
	
		func_neuron_whole_kxi(p_in, p_wei_in, p_ou, n, ix,kx,ki,group_base);
		p_in += xmap;
		p_wei_in += wmap;
	}
}

// func_neuron_whole_oxy, first one x-axis, then loop the y-axis,
// stride = 1, not take the stride into accout,
// note, kernel weight buffer have been re-organized,i.e., weight[ky][ki/n][kx][n], inX[ky][ki/n][x][n]
// out:[oy][ox], parameters in struct str_calc_para, including, kx,ki,ky,ox,oy,mod:0,valid,1,same,2,0
// note, the output is [ko][oy][ox], not [oy][ko/n][ox][n],?? if as next cnn. but if fcn, then?
// So, after using this function, should using re-arrange to do next calculation
// ? normally, inX:[ki][y][x], weight[co][ki][ky][kx]; or [ki][x][y], inX same as weight
// Also, should init the outX first outside this function
void func_neuron_whole_oxy(char *inX, float*p_wei, float *outX, neu_unit_para *p_para)
{
	int kidx,oyidx;
//	int oxidx;
	float *p_wei_in, *p_wei_co;
	char *p_in, *p_in_y;
	float *p_ou, *p_ou_xy;
	int xmap,wmap;
	int n, n_group, kx, ki, ky, ko, Ox,Oy, Ix;
	int Ix2,Ox2;
//	int Iy;
	n = p_para->calc_n;
	n_group = p_para->calc_group;
	ki = p_para->calc_ki;
	Ix = p_para->calc_ix;
	Ox = p_para->calc_ox;
	Oy = p_para->calc_oy;
	
	kx = p_para->unit_calc_para.Kx;
	ky = p_para->unit_calc_para.Ky;
	ko = p_para->unit_calc_para.Co;
	
	p_in = inX;
	p_ou = outX;
	p_wei_in = p_wei;
	xmap = Ox*Oy;
	wmap = ky*kx*ki*n;

	Ix2 = Ix * n*ki;
	Ox2 = Ox;
	
	ASSERT2(Ix,kx,"func_neuron_whole_oxy");

	for(kidx =0; kidx < ko; kidx++)
	{
		p_in_y 		= p_in;
		p_wei_co 	= p_wei_in;
		p_ou_xy		= p_ou;
		
		p_wei_in 	+= wmap;
		// p_in		+= 0; // not change
		p_ou		+= xmap;
		
		for(oyidx=0; oyidx<Oy; oyidx++)
		{
			print_cnn("func_neuron_whole_oxy,[kxidx,oyidx]:[%d/%d,%d/%d],Parameters,[Ix,n,ox,ki,group,Ix2]:[%d,%d,%d,%d,%d,%d], X:\n",kidx,ko,oyidx,Oy,Ix,n,Ox,ki,n_group,Ix2);
			func_neuron_whole_kxiy(p_in_y, p_wei_co, p_ou_xy, n, Ix, kx, ki, ky,n_group);

			// next input and weight
			p_in_y += Ix2;       
			// p_wei_co += 0; // not change
			p_ou_xy += Ox2;
		}
	} // end kidx

	// output, need do stride??
}


// func_neuron_calc_cnn, do convlution,
// stride = 1, not take the stride into accout,
// inX: [C][Y][X], p_wei:[B][C][ky][kx], outX:[B][Oy][Ox]
// outX should be initialized before this function called
// would re-arrage inX and p_wei into weight[ky][ki/n][kx][n], inX[ky][ki/n][x][n], for n=8

// func_neuron_organize, do organize, inX should be different with ouX, n=8
// type:0, char, 1:float, 2:inverse char, 3:inverse float
// input:[si][sy][sx], output:[sy][si/n][sx][n]
void func_neuron_organize(void *inX, void*ouX, int type, int sx, int sy, int si, int n, int *calc_ki)
{

#if 0
	char  *p_char;
	char  *p_outc;
	float *p_float;
	float *p_outf;

	uLint_t *p_conv_tbl;

	int xidx, yidx, kidx,nidx,nsidx, nseg,nrem, ntot;
	
	if (n<1)
	{
		printf("In func_neuron_organize, parameter n should >=1, now is %d, debug\n",n);
	}
	nseg = si/n;
	nrem = si%n;
	ntot = n + nrem;
	*calc_ki = ntot;

	p_char = (char *)inX;
	p_outc = (char *)ouX;
	if (nrem > 0)
	{
		for(yidx=0;yidx<sy;yidx++)
		{
			for(nsidx=0; nsidx<nseg; nsidx++)
			{
				for(xidx=0; xidx<sx; xidx++)
				{
					for(nidx=0; nidx<n; nidx++)
					{
						p_outc[yidx*ntot*sx+nidx*sx*n+xidx*n+nidx] = p_char[(nsidx*n+nidx)*sy*sx+sy*yidx+xidx];
					}
				}
			}
		}
	}
	else
	{
		for(yidx=0;yidx<sy;yidx++)
		{
			for(nsidx=0; nsidx<nseg; nsidx++)
			{
				for(xidx=0; xidx<sx; xidx++)
				{
					for(nidx=0; nidx<n; nidx++)
					{
						p_outc[yidx*ntot*sx+nidx*sx*n+xidx*n+nidx] = p_char[(nsidx*n+nidx)*sy*sx+sy*yidx+xidx];
					}
				}
			}
		}
	}

#endif

}

void func_neuron_calc_cnn(char *inX, float*p_wei, float *outX, neu_unit_para *p_para)
{
#if 0
	int kidx, oxidx,oyidx;
	float *p_wei_in, *p_wei_co;
	char *p_in, *p_in_y;
	float *p_ou, *p_ou_xy;
	int xmap,wmap;
	int n, n_group, kx, ki, ky, ko, Ox,Oy, Ix,Iy;
	
	n = p_para->calc_n;
	n_group = p_para->calc_group;
	ki = p_para->calc_ki;
	kx = p_para->unit_calc_para.Kx;
	ky = p_para->unit_calc_para.Ky;
	ko = p_para->unit_calc_para.Co;
	Ox = p_para->unit_calc_para.Ox;
	Oy = p_para->unit_calc_para.Oy;
	Ix = p_para->unit_calc_para.Ix;
	Iy = p_para->unit_calc_para.Iy;
	

	
	p_in = inX;
	p_ou = outX;
	p_wei_in = p_wei;
	xmap = Ox*Oy;
	wmap = ky*kx*ki;
	
	ASSERT2(kx,n_group,"func_neuron_whole_oxy");

	for(kidx =0; kidx < ko; kidx++)
	{
		p_in_y 		= p_in;
		p_wei_co 	= p_wei_in;
		p_ou_xy		= p_ou;
		
		p_wei_in 	+= wmap;
		// p_in		+= 0; // not change
		p_ou		+= xmap;
		
		for(oyidx=0; oyidx<Oy; oyidx++)
		{
			func_neuron_whole_kxiy(p_in_y, p_wei_co, p_ou_xy, n, n_group, kx, ki, ky);

			// next input and weight
			p_in_y += Ix;       
			// p_wei_co += 0; // not change
			p_ou_xy += Ox;
		}
	} // end kidx
#endif
}

neu_unit_para::neu_unit_para()
{
	neu_unit_para_base_init();
}

neu_unit_para::~neu_unit_para()
{
	if(init_flg != 0)
	{
		neu_release_buf();
		
		init_flg = 0;
	}

}

void neu_unit_para::neu_unit_para_base_init(void)
{
	init_flg= 0;
	neu_unit_para_int();
}

void neu_unit_para::neu_unit_para_int(void)
{
	if(init_flg != 0)
	{
		neu_release_buf();
		
		init_flg = 0;
	}
	else
	{
		NULL_POINT(p_organize_tbl);
		NULL_POINT(p_org_wei_tbl);
		NULL_POINT(p_in_buffer);
		NULL_POINT(p_weight);
		NULL_POINT(p_in_extend_buffer);
		NULL_POINT(p_inner_buffer);
		NULL_POINT(p_out_buffer);
	}
}

void neu_unit_para::neu_release_buf(void)
{
	FREE_POINT(p_organize_tbl);
	FREE_POINT(p_org_wei_tbl);
	FREE_POINT(p_in_buffer);
	FREE_POINT(p_weight);
	FREE_POINT(p_in_extend_buffer);
	FREE_POINT(p_inner_buffer);
	FREE_POINT(p_out_buffer);

}

void neu_unit_para::neu_unit_set_para(str_calc_para *p_para,int type, int n, int n_group, NeuCoreMode neu_md)
{
	printf("neu_unit_set_para begin\n");
	neu_unit_para_int();

	memcpy(&unit_calc_para,p_para,sizeof(str_calc_para));
	neu_mode 	= neu_md;
	calc_type = type;
	calc_x_ah = 0;
	calc_y_ah = 0;
	if (calc_type == 1) // same need exceed, this time not support 1 and 2, only support 0
	{
		calc_ix = unit_calc_para.Ix + unit_calc_para.Kx-1;
		calc_iy = unit_calc_para.Iy + unit_calc_para.Ky-1;
		calc_x_ah = unit_calc_para.Kx/2;
		calc_y_ah = unit_calc_para.Ky/2;
	}
	else if (calc_type == 2) // full need exceed, this is not supported! 
   {
	   calc_ix = unit_calc_para.Ix + 2*(unit_calc_para.Kx-1);
	   calc_iy = unit_calc_para.Iy + 2*(unit_calc_para.Ky-1);
	   calc_x_ah = unit_calc_para.Kx-1;
	   calc_y_ah = unit_calc_para.Ky-1;
   }
   else
   {
		calc_ix = unit_calc_para.Ix;
		calc_iy = unit_calc_para.Iy;
	}

	if (n<1)
	{
		printf("In neu_unit_set_para, parameter n should >=1, now is %d, debug\n",n);
		n = 8;
	}	
	
	calc_ox = calc_ix - unit_calc_para.Kx+1;
	calc_oy = calc_iy - unit_calc_para.Ky+1;

	calc_n = n;
	calc_group = n_group;
	
	calc_ki = (unit_calc_para.Ci + (calc_n-1)) / calc_n;
	// calc_ktot should be equal to calc_ki*calc_n
	calc_tbl_len = calc_ki*calc_n*calc_ix*calc_iy;
	calc_wei_size = unit_calc_para.Kx*unit_calc_para.Ky*calc_ki*calc_n; //*unit_calc_para.Co;

	calc_in_size = calc_ix*calc_iy*calc_ki*calc_n; // = calc_tbl_len
	calc_inner_size = calc_ox*calc_oy*unit_calc_para.Co;
	calc_out_size   = unit_calc_para.Ox*unit_calc_para.Oy*unit_calc_para.Co;
	
	p_organize_tbl 	= (uLint_t *)malloc(sizeof(uLint_t)*calc_tbl_len);
	p_in_buffer 	= (char *)malloc(sizeof(char)*calc_in_size);
	p_inner_buffer 	= (float *)malloc(sizeof(float)*calc_inner_size);
	p_out_buffer 	= (float *)malloc(sizeof(float)*calc_out_size);
	p_in_extend_buffer 	= (char *)malloc(sizeof(char)*calc_in_size);
	p_weight 	= (float *)malloc(sizeof(float)*(calc_wei_size*unit_calc_para.Co));
	p_org_wei_tbl 	= (uLint_t *)malloc(sizeof(uLint_t)*calc_wei_size);

	if ((NULL == p_organize_tbl) || (NULL == p_in_buffer) || (NULL == p_inner_buffer)||(NULL == p_out_buffer) || (NULL==p_in_extend_buffer) || (NULL==p_org_wei_tbl))
	{
		printf("In,neu_unit_set_para,memory is not enough, debug\n Adderess: tble,tbl_wei, input,in_extend,inner,out=%p,%p,%p,%p,%p,%p\n",\
			p_organize_tbl,p_org_wei_tbl,p_in_buffer,p_in_extend_buffer,p_inner_buffer,p_out_buffer);
		neu_release_buf();
		return;
	}

	printf("neu_unit_set_para End\n");

	printf("neu_unit_set_para Input Table:\n");
	func_gen_reorder_tbl(p_organize_tbl, calc_ix, calc_iy, unit_calc_para.Ci, calc_n, &calc_ktot);
	if ((calc_ki*calc_n) != calc_ktot)
	{
		printf("neu_unit_set_para Input Table Error, debug:\n");
	}
	printf("neu_unit_set_para Weight Table:\n");
	func_gen_reorder_tbl(p_org_wei_tbl, unit_calc_para.Kx, unit_calc_para.Ky, unit_calc_para.Ci, calc_n, &calc_wei_tot);
	if ((calc_ki*calc_n) != calc_wei_tot)
	{
		printf("neu_unit_set_para Weight Table Error, debug:\n");
	}
	
	// init memory
	memset(p_in_buffer,0,sizeof(char)*calc_in_size);
	memset(p_inner_buffer,0,sizeof(float)*calc_inner_size);
	memset(p_out_buffer,0,sizeof(float)*calc_out_size);
	memset(p_in_extend_buffer,0,sizeof(char)*calc_in_size);
	
	init_flg = 1;
	
	printf("neu_unit_set_para End\n");
}

// input:[si][sy][sx], output:[sy][si/n][sx][n]
void neu_unit_para::neu_unit_reorder_input(char *inX)
{
	uLint_t idx;
	char *p_in,*p_inner;
	uLint_t *p_tbl;
	
	uLint_t real_in_len, rem_len;
	real_in_len = calc_ix*calc_iy*unit_calc_para.Ci;
	rem_len = calc_ix*calc_iy*(calc_ktot - unit_calc_para.Ci);

	p_in = inX;
	if ((calc_type == 1) || (calc_type == 2))  // same or full, actuallly not support
	{
		int xidx,yidx,cidx;
		char *p_inner_x,*p_inner_y,*p_inner_c,*p_in_x,*p_in_y,*p_in_c;
		uLint_t inner_xy, in_xy;
		memset(p_in_extend_buffer,0,sizeof(char)*calc_in_size);
		p_inner 	= p_in_extend_buffer;
		inner_xy 	= calc_ix*calc_iy;
		in_xy 		= unit_calc_para.Ix*unit_calc_para.Iy;


		// ignore first and last calc_y_ah to padding, here example padding to zeros
		#if 0
		for(cidx=0;cidx<unit_calc_para.Ci;cidx++) // input:[si][sy][sx],
		{
			for(yidx=0;yidx<calc_y_ah;yidx++)
			{
				for(xidx=0;xidx<calc_ix;xidx++)
				{
					p_inner[cidx*inner_xy+yidx*calc_ix+xidx] = 0;
				}

			}
			for(yidx=calc_y_ah+unit_calc_para.Iy;yidx<calc_iy;yidx++)
			{
				for(xidx=0;xidx<calc_ix;xidx++)
				{
					p_inner[cidx*inner_xy+yidx*calc_ix+xidx] = 0;
				}

			}
		}
		#endif
		
		p_inner = p_in_extend_buffer + calc_y_ah*calc_ix; 
		for(cidx=0;cidx<unit_calc_para.Ci;cidx++) // input:[si][sy][sx],
		{
			p_in_c 		= p_in;
			p_inner_c 	= p_inner;
			p_inner		+= inner_xy;
			p_in		+= in_xy;
			
			for(yidx=0;yidx<unit_calc_para.Iy;yidx++)
			{
				p_in_y 		= p_in_c;
				p_inner_y 	= p_inner_c;
				p_in_c 		+= unit_calc_para.Ix;
				p_inner_c	+= calc_ix;
				for(xidx=0;xidx<calc_x_ah;xidx++) // first calc_x_ah set to zeros
				{
					*p_inner_y = 0;
					p_inner_y++;
				}
				p_inner_x = p_inner_y;
				p_in_x	  = p_in_y;
				p_inner_y += unit_calc_para.Ix;
				for(xidx=0;xidx<unit_calc_para.Ix;xidx++)
				{
					p_inner_x[0] = p_in_x[0];
					p_inner_x++;
					p_in_x++;
				}
				for(;xidx<calc_ix;xidx++) // last calc_x_ah set to zeros
				{
					*p_inner_y = 0;
					p_inner_y++;
				}
			}
		}

		// finally, set p_in to p_in_extend
		p_in = p_in_extend_buffer;
	}
	else
	{
		if (unit_calc_para.Ci == calc_ktot)
		{
			p_in = inX;
		}
		else
		{
			// for debug, input reorder
			print_cnn("In input reorder,parameters,[ci,calc_ktot,real_len,rem_len]:[%d,%d,%lu,%lu]\n",unit_calc_para.Ci,calc_ktot,real_in_len, rem_len);
			
			memset(&p_in_extend_buffer[real_in_len],0,sizeof(char)*rem_len);
			memcpy(p_in_extend_buffer,inX,real_in_len);
			p_in = p_in_extend_buffer;
			
			// for debug, input reorder, inner buf
			print_cnn("In input reorder, inner buffer:\n");
			for(idx=0;idx<calc_in_size;idx++)
				print_cnn("%d ",p_in_extend_buffer[idx]);
			print_cnn("\n");

		}
	}

	// for debug, input reorder
	print_cnn("In input reorder, data in:\n");
	for(idx=0;idx<uLint_t(calc_ix*calc_iy*unit_calc_para.Ci);idx++)
		print_cnn("%d ",inX[idx]);
	print_cnn("\n");

	
	// do re-order
	p_tbl = p_organize_tbl;
	for(idx=0; idx<calc_in_size;idx++)
	{
		p_in_buffer[*p_tbl] = *p_in;
		p_tbl++;
		p_in++;
	}

	
	// for debug, input reorder
	print_cnn("After input reorder, data out:\n");
	for(idx=0;idx<calc_in_size;idx++)
		print_cnn("%d ",p_in_buffer[idx]);
	print_cnn("\n");

}

// input:[co][si][ky][kx], output:[co][ky][si/n][kx][n]
void neu_unit_para::neu_unit_reorder_weight(float *p_wei)
{
	uLint_t idx;
	int coidx;
	float *p_in,*p_inner;
	float *p_in_c,*p_inner_c;
	uLint_t *p_tbl;
	uLint_t inner_xy, in_xy, inner_xyc, in_xyc;
	p_in 	= p_wei;
	p_inner = p_weight;
	inner_xy	= unit_calc_para.Kx*unit_calc_para.Ky; // Kx should <= unit_calc_para.Ix and same for Y
	in_xy		= inner_xy;
	inner_xyc	= inner_xy * calc_ktot;
	in_xyc		= in_xy * unit_calc_para.Ci;
	
	print_cnn("Debug in neu_unit_reorder_weight,[Ci,calc_wei_tot]:=[%d,%d]\n",unit_calc_para.Ci,calc_wei_tot);
	
	if (unit_calc_para.Ci != calc_wei_tot)
	{
		for(coidx=0; coidx<unit_calc_para.Co; coidx++)
		{
			p_tbl = p_org_wei_tbl;
			p_inner_c = p_inner;
			p_inner += inner_xyc; // += calc_wei_size;
			p_in_c	 = p_in;
			p_in 	+= in_xyc;

			// first, set all to zeros
			for(idx=0; idx<inner_xyc;idx++)
			{
				p_inner_c[idx] = 0;
			}

			// reorder
			for(idx=0; idx<in_xyc;idx++)
			{
				p_inner_c[*p_tbl] = *p_in_c;
				p_tbl++;
				p_in_c++;
			}
			
		}
	}
	else
	{
		// do re-order
		p_tbl = p_org_wei_tbl;
		p_in = p_wei;
		p_inner = p_weight;
		for(coidx=0; coidx<unit_calc_para.Co; coidx++)
		{
			p_tbl = p_org_wei_tbl;
			p_inner_c = p_inner;
			p_inner += inner_xyc; // += calc_wei_size;
			p_in_c	 = p_in;
			p_in 	+= in_xyc;
			for(idx=0; idx<in_xyc;idx++) // in_xyc = inner_xyc = calc_wei_size
			{
				// debug
				print_cnn("Weight re-order,[co,idx,tbl,in]:[%d,%lu,%lu,%f]\n",coidx,idx,*p_tbl,*p_in_c);
			
				p_inner_c[*p_tbl] = *p_in_c;
				p_tbl++;
				p_in_c++;
			}
			
			// debug
			for(idx=0; idx<in_xyc;idx++) // in_xyc = inner_xyc = calc_wei_size
			{
				print_cnn("Weight re-order,out,[idx,out_va]:[%lu,%f]\n",idx,p_inner_c[idx]);
			}
		}
	}
}

void neu_unit_para::neu_unit_calc_core(void *inX, void *outX)
{
	float *p_inf, *p_ouf,*p_wei;
	char  *p_inc;
	// first, parameters should have been inited 
	// weight has been initialized outside the function

	// then re-order the data
	if (neu_mode == NEU_MODE_A)
	{
		p_inc = (char *)inX;
		neu_unit_reorder_input(p_inc);
		p_inc = p_in_buffer;
		p_wei = p_weight;
		p_ouf = p_out_buffer;
		// do calculate
		func_neuron_whole_oxy(p_inc, p_wei, p_ouf, this);

		// copy to outX
		p_ouf = (float *)outX;
		for(uLint_t idx=0; idx<calc_out_size; idx++)
		{
			p_ouf[idx] = p_out_buffer[idx];
		}
	}
	else
	{
		p_inf = (float *)inX;
		printf("In neu_unit_calc_core, Current Not supported, mode=%d, inX Addr = %p\n",neu_mode,p_inf);
	}

	// output

}


// func_neuron_organize, do organize, inX should be different with ouX, n=8
// type:0, char, 1:float, 2:inverse char, 3:inverse float
// input:[si][sy][sx], output:[sy][si/n][sx][n]
#define DEB_REORD_TBL	1
void func_gen_reorder_tbl(uLint_t *p_tbl, int sx, int sy, int si, int n, int *n_ki)
{
	int xidx, yidx, nidx,nsidx, nseg,nrem, ntot;
	uLint_t *p_tmp;
	uLint_t ad_y, ad_seg, ad_x, ad_xy;
	uLint_t ord_y, ord_seg, ord_x, ord_xy;
	uLint_t xy,nx, nxy;
	nseg = si/n;
	nrem = si%n;
	ntot = (nrem==0)?si:((nseg+1)*n);
	*n_ki = ntot;
	p_tmp = p_tbl;
#if (1 == DEB_REORD_TBL)
	int test_flg;
	test_flg = 1;
	print_cnn("In func_gen_reorder_tbl, table test start\n");
#endif

	if (nrem > 0)
	{
		for(yidx=0;yidx<sy;yidx++)
		{
			for(nsidx=0; nsidx<nseg; nsidx++)
			{
				for(xidx=0; xidx<sx; xidx++)
				{
					for(nidx=0; nidx<n; nidx++)
					{
						// // input:[si][sy][sx], output:[sy][si/n][sx][n], si=ntot=nseg*n
						p_tmp[(nsidx*n+nidx)*sy*sx+sx*yidx+xidx] = yidx*ntot*sx+nsidx*sx*n+xidx*n+nidx;
					}
				}
			}
			nsidx = nseg;
			{
				for(xidx=0; xidx<sx; xidx++)
				{
					for(nidx=0; nidx<nrem; nidx++)
					{
						p_tmp[(nsidx*n+nidx)*sy*sx+sx*yidx+xidx] = yidx*ntot*sx+nsidx*sx*n+xidx*n+nidx;
					}
					for(nidx=nrem; nidx<n; nidx++)
					{
						p_tmp[(nsidx*n+nidx)*sy*sx+sx*yidx+xidx] = yidx*ntot*sx+nsidx*sx*n+xidx*n+nidx;
					}
				}
			}
			
		}
#if (1 ==  DEB_REORD_TBL)
		// compare, for debug
		print_cnn("Reorder Table debug -2,parameters [sx, sy, si, n,ntot,nseg,nrem]:[%d,%d,%d,%d,%d,%d,%d]\n",sx, sy, si, n,ntot,nseg,nrem);
		uLint_t pdidx,pdidx2;
		pdidx2 = 0;
		for(yidx=0;yidx<sy;yidx++)
		{
			for(nsidx=0; nsidx<=nseg; nsidx++)
			{
				for(xidx=0; xidx<sx; xidx++)
				{
					for(nidx=0; nidx<n; nidx++)
					{
						pdidx = yidx*ntot*sx+nsidx*sx*n+xidx*n+nidx;
						if (pdidx != pdidx2 )
						{
							print_cnn("Error func_gen_reorder_tbl, idx not match, get %lu while need %lu, DEBUG \n",pdidx, pdidx2);
						}
						print_cnn("%lu ",p_tbl[pdidx]);
						if ((pdidx % 10 )== 9)
							print_cnn("\n");
						print_cnn("\n");

						pdidx2++;
					}
				}
			}
		}
		print_cnn("Reorder Table debug -2 End\n");

#endif

		
	}
	else 	// input:[si][sy][sx], output:[sy][si/n][sx][n], si=ntot=nseg*n
	{
		ord_y = 0;
		xy 	= sx*sy;
		nx = n*sx;
		nxy = n*xy;
		ad_y = 0;

		#if (1 == DEB_REORD_TBL)
		// for test
		uLint_t *p_test_tbl;
		uLint_t tbl_test_len;
		tbl_test_len = ntot*sx*sy;
		p_test_tbl = (uLint_t *)malloc(sizeof(uLint_t)*tbl_test_len);
		#endif
		
		for(yidx=0;yidx<sy;yidx++)
		{
			ord_seg = ord_y;  // output
			ord_y 	+=  ntot*sx;
			ad_seg   = ad_y; // input-index
			ad_y	+= sx;
			for(nsidx=0; nsidx<nseg; nsidx++)
			{
				ord_x = ord_seg;
				ord_seg	+= nx;
				ad_x   = ad_seg;
				ad_seg += nxy;
				for(xidx=0; xidx<sx; xidx++)
				{
					ord_xy = ord_x; // output
					ord_x += n;
					ad_xy  = ad_x;  // input-index
					ad_x  += 1;
					p_tmp = &p_tbl[ad_xy];
					for(nidx=0; nidx<n; nidx++)
					{
						p_tmp[0] = ord_xy;  // output
						ord_xy += 1;
						p_tmp  += xy;        // input-index
						// // input:[si][sy][sx], output:[sy][si/n][sx][n], si=ntot=nseg*n
						//p_tbl[ (nsidx*n+nidx)*sy*sx+sx*yidx+xidx]=  yidx*ntot*sx+nsidx*sx*n+xidx*n+nidx;
						p_test_tbl[ (nsidx*n+nidx)*sy*sx+sx*yidx+xidx]=  yidx*ntot*sx+nsidx*sx*n+xidx*n+nidx;
					}
				}
			}
		}

		#if (1 ==  DEB_REORD_TBL)
		// compare, for debug
		print_cnn("Reorder Table debug,parameters [sx, sy, si, n]:[%d,%d,%d,%d]\n",sx, sy, si, n);
		for(uLint_t tidx=0; tidx<tbl_test_len; tidx++)
		{
			if ((p_test_tbl[tidx] - p_tbl[tidx]) != 0)
			{
				test_flg = 0;
				print_cnn("In func_gen_reorder_tbl, table not correct, debug, idx=%lu,test_tbl:%lu, tbl2:%lu\n",tidx,p_test_tbl[tidx],p_tbl[tidx]);
				break;
			}
			print_cnn("%lu ",p_test_tbl[tidx]);
			if ((tidx % 10 )== 9)
				print_cnn("\n");
			print_cnn("\n");
		}
		print_cnn("Reorder Table debug End\n");

		FREE_POINT(p_test_tbl);
		#endif
		
	}	
	
	#if (1 ==  DEB_REORD_TBL)
	if (test_flg == 1)
	{
		print_cnn("In func_gen_reorder_tbl, table test SUCCESS!\n");
	}
	#endif
}


///////////// A Test Example
 
#define CI_TEST 	(7)
#define CO_TEST 	(11)
#define IX_TEST 	(16)
#define IY_TEST 	(16)
#define KX_TEST 	(3)
#define KY_TEST 	(3)
#define OX_TEST 	(IX_TEST-KX_TEST+1)
#define OY_TEST 	(IY_TEST-KY_TEST+1)
			
void test_neu_core_calc(void)
{
	neu_unit_para *p_test_neu_core;

	p_test_neu_core = (neu_unit_para *)malloc(sizeof(neu_unit_para));
	if (NULL == p_test_neu_core)
	{
		printf("Error in test_neu_core_calc, could not malloc neu_unit_para for test, debug\n");
		return;
	}
	p_test_neu_core->neu_unit_para_base_init();
	
	str_calc_para calc_para0;
	int clac_type; //0:valid, 1:same, 2:full
	int calc_n,calc_ci,calc_co;
	int calc_group;
	int cidx,xidx,yidx,coidx;
	
	NeuCoreMode neu_mode;  // 
	neu_mode = NEU_MODE_A;
	calc_n = 9;
	calc_ci = CI_TEST;
	calc_co = CO_TEST;
	calc_group = 5;
	clac_type = 0;
	calc_para0.Ci = calc_ci;
	calc_para0.Co = calc_co;
	calc_para0.Ix = IX_TEST;
	calc_para0.Iy = IY_TEST;
	calc_para0.Ox = OX_TEST;
	calc_para0.Oy = OY_TEST;
	calc_para0.Kx = KX_TEST;
	calc_para0.Ky = KY_TEST;
	calc_para0.stride_x = 1;
	calc_para0.stride_y = 1;
	calc_para0.bias_en	= 0;
	calc_para0.size_out	= calc_para0.Ox*calc_para0.Oy*calc_para0.Co;


		char out_pos[5] = {10,11,12,13,14};
		int out_len = 1;
	
		int seed_16 = 0x0B6A9;
		rand_cla rand_ker;
		char rand_out[16];
		int mem_data,pos_data;
		
		rand_ker.rand_config_pos(out_len,out_pos);
		rand_ker.rand_sta_set_int(seed_16,0);


		

	// init test-data,[si][sy][sx]
	char in_buffer[CI_TEST][IY_TEST][IX_TEST];
	for(cidx=0; cidx<calc_para0.Ci; cidx++)
	{
		for(yidx=0; yidx<calc_para0.Iy; yidx++)
		{
			for(xidx=0; xidx<calc_para0.Ix; xidx++)
			{
//				in_buffer[cidx][yidx][xidx] = (cidx+yidx+xidx)&0x0001;
				rand_ker.rand_pro(rand_out);
				rand_ker.rand_read_mem_data(&mem_data, &pos_data);
				in_buffer[cidx][yidx][xidx] = pos_data & 0x0001;
			}
		}
	}

	// init output-data
	float out_buffer[CO_TEST][OY_TEST][OX_TEST];
	for(cidx=0; cidx<calc_para0.Co; cidx++)
	{
		for(yidx=0; yidx<calc_para0.Oy; yidx++)
		{
			for(xidx=0; xidx<calc_para0.Ox; xidx++)
			{
				out_buffer[cidx][yidx][xidx] = 0;
			}
		}
	}

	// init kernel
	float ker_buffer[CO_TEST][CI_TEST][KY_TEST][KX_TEST];
	for(coidx=0; coidx<calc_para0.Co; coidx++)
	{
		for(cidx=0; cidx<calc_para0.Ci; cidx++)
		{
			for(yidx=0; yidx<calc_para0.Ky; yidx++)
			{
				for(xidx=0; xidx<calc_para0.Kx; xidx++)
				{
					ker_buffer[coidx][cidx][yidx][xidx] = ((coidx*calc_para0.Ci+cidx)*calc_para0.Ky+yidx)*calc_para0.Kx+xidx;
				}
			}
		}
	}
	
	calc_para0.p_weight = &ker_buffer[0][0][0][0];
	calc_para0.p_bias	= NULL;
	calc_para0.bias_en	= 0;
	p_test_neu_core->neu_unit_set_para(&calc_para0,clac_type, calc_n, calc_group,neu_mode);


	// get expected data
	float out_buffer2[CO_TEST][OY_TEST][OX_TEST];
	uLint_t erridx =0;
	int errnum = 0;
	
	print_cnn("In test_neu_core_calc, get expected data\n");
	memset(out_buffer2,0,sizeof(float)*CO_TEST*OY_TEST*OX_TEST);
	func_cnn_spike_pro(&in_buffer[0][0][0], &out_buffer2[0][0][0], &calc_para0);
	// End get expected data



	// do weight organize
	print_cnn("In test_neu_core_calc, re-order weight Start\n");
	p_test_neu_core->neu_unit_reorder_weight(&ker_buffer[0][0][0][0]);

	// debug print weight
	print_cnn("Debug,Input before re-order:\n");
	print_cnn("[ci,y,x,input]\n");
		for(cidx=0; cidx<calc_para0.Ci; cidx++)
		{
			for(yidx=0; yidx<calc_para0.Iy; yidx++)
			{
				for(xidx=0; xidx<calc_para0.Ix; xidx++)
				{
					print_cnn("%d ",in_buffer[cidx][yidx][xidx]);
				}
				print_cnn("\n");
			}
		}


	print_cnn("Debug,weight before re-order:\n");
	for(coidx=0; coidx<calc_para0.Co; coidx++)
	{
		for(cidx=0; cidx<calc_para0.Ci; cidx++)
		{
			for(yidx=0; yidx<calc_para0.Ky; yidx++)
			{
				for(xidx=0; xidx<calc_para0.Kx; xidx++)
				{
					print_cnn("[co,ci,y,x,wei]:[%d,%d,%d,%d,%f]\n",coidx,cidx,yidx,xidx,ker_buffer[coidx][cidx][yidx][xidx]);
				}
			}
		}
	}
	print_cnn("Debug,weight after re-order,parameter,[co,wei_size]:[%d,%lu]\n",calc_para0.Co,p_test_neu_core->calc_wei_size);
	for(coidx=0; coidx<calc_para0.Co; coidx++)
	{
		for(uLint_t i0=0; i0<p_test_neu_core->calc_wei_size;i0++)
		{
			print_cnn("[co,ci,wei_out]:[%d,%lu,%f]\n",coidx,i0,p_test_neu_core->p_weight[coidx*p_test_neu_core->calc_wei_size+i0]);
		}
	}
	
	print_cnn("In test_neu_core_calc, re-order weight End\n");
	// End debug print weight

	// do calculate
	print_cnn("In test_neu_core_calc,Calculate Start\n");
	p_test_neu_core->neu_unit_calc_core(in_buffer,out_buffer);
	print_cnn("In test_neu_core_calc,Calculate End\n");


	// for debug
	print_cnn("In test_neu_core_calc, get expected data\n");
	memset(out_buffer2,0,sizeof(float)*CO_TEST*OY_TEST*OX_TEST);
	func_cnn_spike_pro(&in_buffer[0][0][0], &out_buffer2[0][0][0], &calc_para0);


	// do compare
	
	print_cnn("In test_neu_core_calc, Test Start\n");
	

	
	print_cnn("In test_neu_core_calc, now compare ...\n");
	for(coidx=0; coidx<calc_para0.Co; coidx++)
	{
			for(yidx=0; yidx<calc_para0.Oy; yidx++)
			{
				for(xidx=0; xidx<calc_para0.Ox; xidx++)
				{
					if (ABS(out_buffer2[coidx][yidx][xidx]-out_buffer[coidx][yidx][xidx])>1e-4)
					{
						errnum++;
						erridx = coidx*(calc_para0.Ox*calc_para0.Oy)+yidx*(calc_para0.Ox)+xidx;
						printf("Error,not match,idx:%lu,[%d,%d,%d],calc:%f, expected:%f\n",erridx,coidx,yidx,xidx,out_buffer[coidx][yidx][xidx],out_buffer2[coidx][yidx][xidx]);
					}

					printf("output:[%d,%d,%d],results:%f,expected:%f\n",coidx,yidx,xidx,out_buffer[coidx][yidx][xidx],out_buffer2[coidx][yidx][xidx]);
				}
			}

	}
	if (errnum == 0)
	{
		printf("In test_neu_core_calc, Test PASS\n");
	}


	FREE_POINT(p_test_neu_core);
}

