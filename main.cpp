#include "stdio.h"


#include "common.h"

#include "calc_util.h"

#include "dvs_process.h"

float g_InBuffer[TEST_MAP_MAX_SIZE];
int g_IdeaBuffer[TEST_LABEL_OUT_MAX_SIZE]; // as Ideal res
float g_OuBuffer[TEST_LABEL_OUT_MAX_SIZE];
char g_InSpikeOne[TEST_MAP_MAX_SIZE];

Spike_generator 	g_spike_generator;

extern FILE *fp_deb_spike_cnn;

int main(void)
{

	Spike_generator *p_spike_gen;
	
	p_spike_gen = &g_spike_generator;
	if(0 != p_spike_gen->spike_gen_init())
	{
		printf("Error in main, spike_gen_init, debug!\n");
		return 0;
	}
    
//	test_rand();
//	test_spike_gen();

	// test dvs2frame
//	test_dvs2frame();

	// test neu_core
	if (NULL == (fp_deb_spike_cnn = fopen("deb_cnn.txt","w")))
	{
		printf("Error in main,fp_deb_spike_cnn could not open file,debug\n");
	}
	test_neu_core_calc();


	printf("Simu End! \n");
	
    return 0;
}
