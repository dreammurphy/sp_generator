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


