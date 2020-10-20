#ifndef __SLT_TYPE_H__
#define __SLT_TYPE_H__


#ifdef WIN32
typedef signed char			int8_t;
typedef signed short		int16_t;
typedef signed int			int32_t;
typedef __int64				int64_t;

typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned int		uint32_t;
typedef unsigned __int64	uint64_t;
#endif


typedef enum{
	SLT_RES_ERR     = -1,
	SLT_RES_OK      = 0,
	SLT_RES_TESTING = 1
} SLT_RESULT;


//
//  1. run 실행 후 반드시 100 msec 이내에 return 되어야 함.
//	2. stop 이후 반드시 100 msec 이내에 return 되어야 함.
//	3. status 실행 이후 즉시 status가 return 되어야 함.
//
typedef struct tag_SLT_TEST_MODULE
{
	char name[64];					//	Module Name
	SLT_RESULT (*run)(void);	//	Run Test
	SLT_RESULT (*stop)(void);	//	Stop Test
	SLT_RESULT (*status)(void);	//	Current Status (ERR,OK,TESTING)
} SLT_TEST_MODULE;


//
//	CPU / 3D / VPU 각각의 Module의 실행파일은 아래와 같은 API 를 extern 하여야 함.
//
extern SLT_TEST_MODULE *GetCpuTestModule(void);
extern SLT_TEST_MODULE *GetVpuTestModule(void);
extern SLT_TEST_MODULE *Get3DTestModule(void);
extern void InitOGLES();
extern void Render();
extern void DeInitOGLES();
extern void TestStart(void);
extern void TestStop();


#if 0	//	CPU Test Application Example
// CPU Test Application Example
SLT_TEST_RESULT cpu_test()
{
	SLT_TEST_MODULE *test = GetCpuTestModule();
	SLT_RESULT res;
	uint32_t count = 10;

	res = test->run();
	if( res < 0 )
		return res;

	while(count-->0)
	{
		usleep(1000000);	//	sleep 1sec
		res = test->status();
		if( res < 0 )
		{
			return res;
		}
	}
	res = test->stop();
	return res;
}
#endif


#endif // __SLT_TYPE_H__
