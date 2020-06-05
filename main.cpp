#include "stdio.h"


#include "common.h"

#include "calc_util.h"


float g_InBuffer[TEST_MAP_MAX_SIZE];
int g_IdeaBuffer[TEST_LABEL_OUT_MAX_SIZE]; // as Ideal res
float g_OuBuffer[TEST_LABEL_OUT_MAX_SIZE];
char g_InSpikeOne[TEST_MAP_MAX_SIZE];

Spike_generator 	g_spike_generator;


int main(void)
{

	Spike_generator *p_spike_gen;
	
	p_spike_gen = &g_spike_generator;
	if(0 != p_spike_gen->spike_gen_init())
	{
		printf("Error in main, spike_gen_init, debug!\n");
		return 0;
	}
    
	test_rand();
	test_spike_gen();

	printf("Simu End! \n");
	
    return 0;
}
