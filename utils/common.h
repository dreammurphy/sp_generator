#ifndef _COMMON_H_
#define _COMMON_H_


#include "stdio.h"
#include "stdlib.h"

// #define NULL						(0) //defined in stdio.h

#define SPIKE_MODE_SEL					2 //0:fix, 1:random 2, ahead
#define CASE_TEST						2 // 0:XOR, 1:MNIST,FCN 2:MNIST CNN
#define CASE_DEBUG						0 // 1:debug

#define TEST_MAP_MAX_SIZE       (1<<24)// 16M
#define TEST_LABEL_OUT_MAX_SIZE    (10)
#define LAYER_MAX_NUMBER			(128) // not using malloc function


#define SIMU_TIME_LEN				(50)

typedef enum
{
    LAYER_FCN = 0,
    LAYER_CNN,
	LAYER_POOLING_AVE,
    LAYER_POOLING_MAX,
    LAYER_DIVERSITY,
    LAYER_OTHERS


} LayerType;

typedef unsigned long int uLint_t;
typedef unsigned int uint_t;


#define FREE_POINT(p)  {if (NULL != (p)) {free((p)); (p)=NULL;}}



/* str_data_para: Origin Data */
typedef struct str_data_para
{
    public:
    char *para=NULL;
	uint_t nx,ny,nf; // input map Size, x, y and features(CI)
	uint_t outf;     // output features(Co)
	uLint_t n_tot;

}str_data_para; 

/* str_judge_data: Origin Data */
typedef struct str_judge_data
{
    public:
//    float *judge_data;
	int judge_data;
    uLint_t err_num=0;
    uLint_t n_tot=0;
    float err_rate;

}str_judge_data; 

typedef struct str_calc_para
{

	float		*p_weight=NULL;
	float		*p_bias=NULL;
	int Ix,Iy,Ci,Ox,Oy,Co,Kx,Ky,stride_x,stride_y;
	uLint_t size_out;
	char bias_en;
} str_calc_para;



#endif
