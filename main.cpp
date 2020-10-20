
#include <stdio.h>
#include <stdlib.h>
#include "slt_type.h"
#include <unistd.h> /* for open/close .. */
#include "Fake_OS.h" /* for OS_GetTickCount */

int main( void )
{
#if 1
	SLT_TEST_MODULE *test = Get3DTestModule();	
	SLT_RESULT res;
	unsigned int test_start_time, test_end_time;	
	int count = 10;
	
	test_start_time = (unsigned int)OS_GetTickCount();	  

	res = test->run();
	if( res < 0 )
		return res;

	while(count-- > 0)
	{
		usleep(1000000);	//	sleep 1sec
		res = test->status();
		if( res != SLT_RES_TESTING)
		{
			return res;
		}
	}
	res = test->stop();
	
	test_end_time = (unsigned int)OS_GetTickCount();	
	printf( "test end - time(%d)\n", test_end_time-test_start_time );
	free(test);
	return 0;
#else	
	int count = 10;
	printf("Test start...\n");
	TestStart();
	printf("TestStart done\n");
	InitOGLES();
	printf("InitOGLES done\n");
	while(count-- > 0)
	{
		Render();
		printf("Render %d done\n", count);
	}
	DeInitOGLES();
	printf("DeInitOGLES done\n");
	TestStop();
	printf("Test ended\n");
#endif
}

