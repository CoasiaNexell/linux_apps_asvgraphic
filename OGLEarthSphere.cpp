// OGLEarthSphere.cpp : Defines the entry point for the application.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* for open/close .. */

#include "OGLEarthSphere.h"
#include <math.h>
#include <GLES/gl.h>
#include <EGL/egl.h>
#include "Fake_OS.h"

#include <semaphore.h> 
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "slt_type.h"
#include "V6_sphere_2000.h"
#include "GLFont.h"

///////////////////////////////////////////////////////////////////////////////
//#define TEST_FEATURE_DISPLAY_INFO
//#define TEST_FEATURE_USE_KEY
#define TEST_FRAMES_MAX					100
#define TEST_GOLDEN_FRAME_INTERVAL		10
#ifdef TEST_FEATURE_DISPLAY_INFO
#define TIME_CHECK_LOOP_CNT				100
#endif

#define TEST_VERSION 1.0f

#define glF(x)	((GLfixed)((x)*(1<<16)))
#define glD(x)	glF(x)
#define GL_F	GL_FIXED
typedef GLfixed GLf;

#define X1_0	65536
#define X0_0	0
#define X5_0	327680
#define X0_5	32768
#define X0_2	13107
#define X0_X	(6553/2)
#define X0_1	6553
#define X0_3	21845
#define X1_2	(X1_0+X0_2)
#define X0_8	(X1_0-X0_2)
#define X0_9	(X1_0-X0_1)

#define LCD_WIDTH  DISP_WIDTH
#define	LCD_HEIGHT DISP_HEIGHT

#define model	V6_sphere_980
#define	model2	V6_sphere_9680

/////////////////////////////////////////////////////////////////////////////////////
#define MAX_LOADSTRING 100

#define VR_MSG	printf
#define VR_ERR	printf

//state
static SLT_RESULT gState = SLT_RES_OK;
static int gRenderTimeoutState = 0;
static int gThreadExit = 0;

//VBO variables
static GLuint nBufferName980[3];
static GLuint nBufferName9680[3];

enum BUFFERTYPE{
	VERTEX_BUFFER,
	INDEX_BUFFER,
	TEXTURECOORD_BUFFER,
};
//VBO variables

///////////////////////////////////////////////////////////////////////////////
static GLfixed g_Perspective[16] = 
{
		glF(2.4142141f), 0, 0, 0,
		0, glF(2.4142141f), 0, 0,
		0, 0, glF(-1.0010005f), glF(-1.0),
		0, 0, glF(-1.0005002f), 0
};
static GLfixed	xrot;							// X Rotation
static GLfixed	yrot;							// Y Rotation
static GLfixed xspeed = X1_0;					// X Rotation Speed
static GLfixed yspeed = X1_0;					// Y Rotation Speed
static GLfixed	g_depth=-X5_0;					// Depth Into The Screen
static GLuint	gTexID1;

static int vtxid = 1;
static int drawMode = GL_TRIANGLES;
static int nfirstTime = 0;
static int nLastTime = 0;
static int nCnt =0;			
static float FrmBuff[100]={ 0x00, };	
static unsigned int nFrames;
static unsigned int* gResultImage;
static unsigned int* gGoldenImage;
static int err;

#ifdef TEST_FEATURE_DISPLAY_INFO
static CGLFont* m_pGLFont;
#endif
static float	fps = 0.0f;			// Holds The Current FPS (Frames Per Second) 

static EGLDisplay glesDisplay;  // EGL display
static EGLSurface glesSurface;	 // EGL rendering surface
static EGLContext glesContext;	 // EGL rendering context

static EGLNativeWindowType hNativeWnd = 0;
static int		view_width	= 0;
static int		view_height	= 0;
static int gDemoMode = 1;
static int gRenderLoopCnt = 1;
static int gDrawEarthCnt = 1;
static EGLNativeDisplayType gEGLDispNative = NULL;

///////////////////////////////////////////////////////////////////////////////
static void GL_Draw(void);
static void InitTestFramework(void);
static void DeInitTestFramework(void);
static GLboolean SaveFrame(unsigned int frame_count);
static GLboolean CheckResult(unsigned int frame_count, FILE* fp100, FILE* fp1_9);
static void glPerspectivef(GLfloat fov, GLfloat aspect, GLfloat near_val, GLfloat far_val);
static void OrthoBegin(void);
static void OrthoEnd(void);
static float framerate(int Poly);
static bool LoadTGA(TGAImage *texture, char *filename);
static double GetTime();
///////////////////////////////////////////////////////////////////////////////
// EGL X
///////////////////////////////////////////////////////////////////////////////
static GLboolean AppeglxCreate (GLint width, GLint height, GLint mode);
static void AppeglxDelete(void);
static void AppeglxFlush(void);
static void AppSetStatus(SLT_RESULT state);
SLT_RESULT AvsGrahpicGetStatus(void);


///////////////////////////////////////////////////////////////////////////
/*
EGLConfig	g_EGLXConfig;
EGLContext	g_EGLXContext;
EGLSurface	g_EGLXSurface;
EGLDisplay	g_EGLXDisplay;
*/
EGLNativeWindowType  m_nativeWindow;

EGLint		g_EGLXNumOfConfigs;
EGLint		g_EGLXAttributeList[] = {       
		EGL_DEPTH_SIZE, 16,  
		EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
		EGL_NONE
};

#if 0
typedef struct tagVRBaseRuntimeThread
{
	pthread_t thread;
	int args[3];
}VRBaseRuntimeThread;
typedef struct tagVRBaseRuntimeThread* HTHREAD;
static HTHREAD hthread;
#else
static pthread_t hthread;
#endif

//---------------------------------------------------------
//    Functions  
//---------------------------------------------------------
void* base_runtime_grahpic_test_thread(void* arg)
{
	unsigned int count = 0;
	printf( "Grahpic processing start, frames(%d)\n", nFrames );

	InitTestFramework();
	
	InitOGLES();

#ifdef TEST_FEATURE_USE_KEY
	while(1)
	{		
		char key = getchar();
		unsigned int obj_offset = 50;
		printf("[KEY]:%c\n", key);
		if('1' == key)
		{
			printf("[KEY 1]:%c inc obj, total(%d)\n", key, vtxid);
			if(gDemoMode < 10) /*0 ~ 10, 11EA */
				gDemoMode++;
			gDrawEarthCnt++;
		}
		else if('2' == key)
		{
			printf("[KEY 1]:%c dec obj, total(%d)\n", key, vtxid);
			vtxid = 1;
			if(gDemoMode)
				--gDemoMode;
			if(gDrawEarthCnt)
				--gDrawEarthCnt;
		}

		if(0 == gDemoMode)
		{
			vtxid = 2;			
		}
		else
		{

			vtxid = 1;
			gRenderLoopCnt = gDemoMode;
		}

		usleep(500000);	
	}
#else
	int err = GL_NO_ERROR;
	char sName[128];
	static int run_count;
	FILE* fp100; 
	FILE* fp1_9;
	unsigned int bytes_size = view_width * 4 * view_height;
	#if 1
	sprintf((char*)sName, "./data/%s_%d_%dx%d_%d.data", "golden", 100, view_width, view_height, 4);
	fp100 = fopen((char*)sName, "r");	
	if(!fp100)
	{
		printf("can't open file(%s)\n", sName);
	}
	sprintf((char*)sName, "./data/%s_%dx%d_%d.data", "golden1-9", view_width, view_height, 4);
	fp1_9 = fopen((char*)sName, "r");	
	if(!fp1_9)
	{
		printf("can't open file(%s)\n", sName);
	}
	#else //temp test
	sprintf((char*)sName, "./data/%s_%d_%dx%d_%d.data", "golden", 100, view_width, view_height, 4);
	fp100 = fopen((char*)sName, "w");	
	if(!fp100)
	{
		printf("can't open file(%s)\n", sName);
	}
	sprintf((char*)sName, "./data/%s_%dx%d_%d.data", "golden1-9", view_width, view_height, 4);
	fp1_9 = fopen((char*)sName, "w");	
	if(!fp1_9)
	{
		printf("can't open file(%s)\n", sName);
	}
	#endif
	#ifdef TEST_FEATURE_DISPLAY_INFO
	m_pGLFont = new CGLFont;
	m_pGLFont->CreateASCIIFont();
	LoadTGA(m_pGLFont->sTgaFontImage,"/c/resources/font_modify.tga");	
	#endif
	
	//Message Loop
	if(!nFrames)
	{
	  while(!gThreadExit)
	  {		  	
		Render();		

		if(SaveFrame(count))
		{
			#if 0 //temp test
			if(count > 0 && count < 10)
			{
				fwrite(gResultImage, bytes_size , 1, fp1_9);
				printf("save count(%d)\n", count);
			}
			else
			{
				fwrite(gResultImage, bytes_size , 1, fp100);
				printf("save count(%d)\n", count);
			}
			#else	
			if(!CheckResult(count, fp100, fp1_9))
			{
				AppSetStatus(SLT_RES_ERR);
				break;
			}
			#endif
		}
	  	++count;
	  }
	}
	else
	{
	  while( !gThreadExit && nFrames-- )
	  {
		Render();
		
		if(SaveFrame(count))
		{
			#if 0 //temp test
			if(count > 0 && count < 10)
			{
				fwrite(gResultImage, bytes_size , 1, fp1_9);
				printf("save count(%d)\n", count);
			}
			else
			{
				fwrite(gResultImage, bytes_size , 1, fp100);
				printf("save count(%d)\n", count);
			}
			#else	
			if(!CheckResult(count, fp100, fp1_9))
			{
				AppSetStatus(SLT_RES_ERR);
				break;
			}
			#endif
		}
		++count;
	  }
	}	

	fclose(fp100);
	fclose(fp1_9);
	usleep(100000); //temp test
#endif	

	DeInitOGLES();

	DeInitTestFramework();
	
	if(AvsGrahpicGetStatus() == SLT_RES_OK)
	{
		printf( "Grahpic processing completed successfully, count(%d)\n", count );
	}
}

void base_runtime_test_mode_default(void)
{
	gDrawEarthCnt = 100;
	vtxid = 1;
}

//---------------------------------------------------------
//    Functions  
//---------------------------------------------------------
#if 0
void* base_runtime_thread_callback(void* pArg)
{
	int* pargument = (int*)pArg;
	void* (*pthreadfunc)(void *) = (void* (*)(void* ))(pargument[0]);
	void* ptemp_arg = (void*)pargument[1];
	HTHREAD hthread = (HTHREAD)pargument[2];
	//VR_DBG("thread callback func start(0x%x), parg(0x%x)\n", (int)pThreadFunc, ptemp_arg);
	pthreadfunc(ptemp_arg);
	return (void*)NULL;
}

//---------------------------------------------------------
//    Functions  
//---------------------------------------------------------
HTHREAD base_runtime_thread_create(void* (*pThreadFunc)(void *), void* pArg)
{
	HTHREAD hthread = (HTHREAD)malloc(sizeof(VRBaseRuntimeThread));
	if( !hthread  ){ return 0; }
	hthread->args[0] = (int)pThreadFunc;
	hthread->args[1] = (int)pArg;
	hthread->args[2] = (int)hthread;
	VR_MSG("[thread] create start\n");
	if( pthread_create(&hthread->thread, NULL, base_runtime_thread_callback, &(hthread->args[0])) < 0 )
	{
		VR_MSG("thread create error\n");
		free(hthread); 
		return 0;
	}
	VR_MSG("[thread] create done\n");
	//VR_DBG("thread create 0x%x, 0x%x, 0x%x\n", hthread->thread, parg[1], parg[2]);
	return (HTHREAD)hthread;
}

//---------------------------------------------------------
//    Functions  
//---------------------------------------------------------
void base_runtime_thread_destroy(HTHREAD hThread)
{
	int* status;
	// 연결된 스레드의 자원을 정상적으로 반환시켜준다
	VR_MSG("[thread] join start\n");
	pthread_join(hThread->thread, (void **)&status);	
	if(0 != status)
	{
		VR_MSG("pthread_join return ERROR!(0x%x)\n", status);
	}
	VR_MSG("[thread] join done\n");
	//at api_gles_runtime_thread_create_SP, doesn't alloc memory for parg
	free(hThread);
}
#endif
///////////////////////////////////////////////////////////////////////////////
SLT_RESULT AvsGrahpicGetStatus(void)
{
	return (!gRenderTimeoutState)? gState : SLT_RES_ERR;
}

void AppSetStatus(SLT_RESULT state)
{
	gState = state;
}

///////////////////////////////////////////////////////////////////////////////
SLT_RESULT AvsGrahpicStart(void)
{
	AppSetStatus(SLT_RES_TESTING);

#if 1
	nFrames = TEST_FRAMES_MAX;
#else
	nFrames = 0;
#endif

	platform_system_init_display_device();

	gEGLDispNative = platform_system_get_native_display();

	hNativeWnd = OS_CreateWindow(); 				  
	view_width = LCD_WIDTH;
	view_height = LCD_HEIGHT;
	if(!hNativeWnd)
	{
		AppSetStatus(SLT_RES_ERR);
		return SLT_RES_ERR;
	}

	/*******************************/
	gThreadExit = 0;
	base_runtime_test_mode_default();

	#if 1
	pthread_create(&hthread, NULL, base_runtime_grahpic_test_thread, NULL);
	#else
	hthread = base_runtime_thread_create(base_runtime_grahpic_test_thread, NULL);
	#endif	
	//printf("AvsGrahpicStart() done\n");
	
	return SLT_RES_OK;
}

///////////////////////////////////////////////////////////////////////////////
SLT_RESULT AvsGrahpicStop()
{	
	printf("AvsGrahpicStop() start\n");
	gThreadExit = 1;

	#if 1
	if(hthread)
	{
		void *exitcode = NULL;
		pthread_join(hthread, &exitcode);	
	}
	#else
	if(hthread)
		base_runtime_thread_destroy(hthread);		
	#endif
	
	OS_DestroyWindow(hNativeWnd);  
	hNativeWnd = NULL;

	platform_system_destory_native_display();
	gEGLDispNative = NULL;
	
	platform_system_deinit_display_device();

	printf("AvsGrahpicStop() done\n");

	return SLT_RES_OK;
}

///////////////////////////////////////////////////////////////////////////////
void TestStart(void)
{
	platform_system_init_display_device();

	gEGLDispNative = platform_system_get_native_display();

	hNativeWnd = OS_CreateWindow(); 				  
	view_width = LCD_WIDTH;
	view_height = LCD_HEIGHT;
	if(!hNativeWnd)
	{
		return ;
	}

	base_runtime_test_mode_default();
}

void TestStop()
{		
	OS_DestroyWindow(hNativeWnd);  
	hNativeWnd = NULL;

	platform_system_destory_native_display();
	gEGLDispNative = NULL;
	
	platform_system_deinit_display_device();
}

///////////////////////////////////////////////////////////////////////////////
SLT_TEST_MODULE *Get3DTestModule(void)
{
/*
char name[64];					//	Module Name
SLT_RESULT (*run)(void);	//	Run Test
SLT_RESULT (*stop)(void);	//	Stop Test
SLT_RESULT (*status)(void); //	Current Status (ERR,OK,TESTING)
*/
	SLT_TEST_MODULE* gragphic_test_module = (SLT_TEST_MODULE*)malloc(sizeof(SLT_TEST_MODULE));
	if(!gragphic_test_module)	
		return NULL;

	strcpy(gragphic_test_module->name, "VR graphic");
	gragphic_test_module->run = AvsGrahpicStart;
	gragphic_test_module->stop = AvsGrahpicStop;
	gragphic_test_module->status = AvsGrahpicGetStatus;
	return gragphic_test_module;
}

///////////////////////////////////////////////////////////////////////////////
static void InitTestFramework(void)
{
	typedef struct SLT_TEST_MODULE
	{
		char name[64];					//	Module Name
		SLT_RESULT (*run)(void);	//	Run Test
		SLT_RESULT (*stop)(void);	//	Stop Test
		SLT_RESULT (*status)(void); //	Current Status (ERR,OK,TESTING)
	} SLT_TEST_MODULE;

	gResultImage = (unsigned int*)malloc(view_width * view_height * 4);
	gGoldenImage = (unsigned int*)malloc(view_width * view_height * 4);
}

static void DeInitTestFramework(void)
{
	free(gResultImage);
	gResultImage = NULL;
	free(gGoldenImage);
	gGoldenImage = NULL;
}

VR_BOOL nx_dbg_image_save_bmp_from_gl_rgba8888(const char *name, unsigned int count, int width, int height, const unsigned char* pixels_in_8bpp) 
{
    unsigned char basic_bmp_header[54] = {
        0x42,0x4d,0x38,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,0x00,0x28,0x00,
        0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x00,0x20,0x00,0x00,0x00,
        0x00,0x00,0x02,0x00,0x10,0x00,0x12,0x0b,0x00,0x00,0x12,0x0b,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00 };
    if ((width <= 0) || (height <= 0) || (!pixels_in_8bpp)) { return VR_FALSE; }
    basic_bmp_header[18] = (width >> 0) & 0xFF;
    basic_bmp_header[19] = (width >> 8) & 0xFF;
    basic_bmp_header[20] = (width >> 16) & 0xFF;
    basic_bmp_header[21] = (width >> 24) & 0xFF;
    basic_bmp_header[22] = (height >> 0) & 0xFF;
    basic_bmp_header[23] = (height >> 8) & 0xFF;
    basic_bmp_header[24] = (height >> 16) & 0xFF;
    basic_bmp_header[25] = (height >> 24) & 0xFF;

	char sName[128];
	sprintf((char*)sName, "%s_%d_%dx%d.bmp", name, count, width, height);
    
    FILE* fp = NULL;
#ifdef _WIN32	// Windows version
    if (fopen_s(&fp, sName, "wb") != 0) {
        printf("can't open (%s)\n", sName);
        return VR_FALSE;
    }
#else		   // Linux version
    fp = fopen(sName, "w");
    if (fp == 0) {
        printf("can't open (%s)\n", sName);
        return VR_FALSE;
    }
#endif
    fwrite(basic_bmp_header, 1, sizeof(basic_bmp_header), fp);	
	unsigned char* pdata_input_rgba = (unsigned char*)malloc(width * height * 4);
	if (!pdata_input_rgba)
	{
        printf("can't alloc\n");
	}
	for (int i = 0; i < width*height; i++) {
		pdata_input_rgba[i * 4 + 2] = pixels_in_8bpp[i * 4 + 0];	
		pdata_input_rgba[i * 4 + 1] = pixels_in_8bpp[i * 4 + 1];	
		pdata_input_rgba[i * 4 + 0] = pixels_in_8bpp[i * 4 + 2];		
	}
    fwrite(pdata_input_rgba, 1, width * height * 4, fp);
    fflush(fp);
    fclose(fp);    
    free(pdata_input_rgba);
    
	printf("file saved: %s\n", sName);
    return VR_TRUE;
}


static GLboolean SaveFrame(unsigned int frame_count)
{
	if( (frame_count > 0 && frame_count < 10) || 
	    (!(frame_count % TEST_GOLDEN_FRAME_INTERVAL)) )
	{
		glReadPixels(0, 0, view_width, view_height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)gResultImage);	
		#if 0
		nx_dbg_image_save_bmp_from_gl_rgba8888("./save/read", frame_count, view_width, view_height, (const unsigned char*)gResultImage);
		#endif
		return GL_TRUE;
	}
	return GL_FALSE;
}

///////////////////////////////////////////////////////////////////////////////
static GLboolean CheckResult(unsigned int frame_count, FILE* fp100, FILE* fp1_9)
{

	FILE* fp = NULL;
	
	if(frame_count > 0 && frame_count < 10)
	{
		fp = fp1_9;
	}
	else if(!(frame_count % TEST_GOLDEN_FRAME_INTERVAL))
	{
		fp = fp100;
	}

	if( fp )
	{	
		unsigned int bytes_size = view_width * 4 * view_height;
		//unsigned int* pdata_rst = (unsigned int*)malloc(bytes_size);
		//load file
		fread(gGoldenImage, bytes_size , 1, fp);
		/*
		if (!frame_count)
		{
			return GL_TRUE;
		}
		*/
		for(unsigned int j = 0 ; j < view_height ; j++)
		for(unsigned int i = 0 ; i < view_width ; i++)
		{
			#if 0
			if((gGoldenImage[j*view_width + i] & 0x00FFFFFF) != (gResultImage[j*view_width + i] & 0x00FFFFFF))
			{
				unsigned char* pdata8_ref = (unsigned char*)&gGoldenImage[j*view_width + i];
				unsigned char* pdata8_rst = (unsigned char*)&gResultImage[j*view_width + i];
				int pass = 1;
				if(pdata8_ref[0] > pdata8_rst[0])
				{
					if((pdata8_ref[0] - pdata8_rst[0]) > 1)
						pass = 0;
				}
				else if(pdata8_ref[0] < pdata8_rst[0])
				{
					if((pdata8_rst[0] - pdata8_ref[0]) > 1)
						pass = 0;
				}
				if(pdata8_ref[1] > pdata8_rst[1])
				{
					if((pdata8_ref[1] - pdata8_rst[1]) > 1)
						pass = 0;
				}
				else if(pdata8_ref[1] < pdata8_rst[1])
				{
					if((pdata8_rst[1] - pdata8_ref[1]) > 1)
						pass = 0;
				}
				if(pdata8_ref[2] > pdata8_rst[2])
				{
					if((pdata8_ref[2] - pdata8_rst[2]) > 1)
						pass = 0;
				}
				else if(pdata8_ref[2] < pdata8_rst[2])
				{
					if((pdata8_rst[2] - pdata8_ref[2]) > 1)
						pass = 0;
				}
				if(pdata8_ref[3] > pdata8_rst[3])
				{
					if((pdata8_ref[3] - pdata8_rst[3]) > 1)
						pass = 0;
				}
				else if(pdata8_ref[3] < pdata8_rst[3])
				{
					if((pdata8_rst[3] - pdata8_ref[3]) > 1)
						pass = 0;
				}

				if(!pass)
				{
					printf("======================================\n");
					printf(" <%d> CheckResult Fail!!!.\n", frame_count);
					printf(" <i:%d, j:%d>ref(0x%08x), out(0x%08x)\n", i, j, gGoldenImage[j*view_width + i], gResultImage[j*view_width + i]);
					printf("======================================\n");
					AppSetStatus(SLT_RES_ERR);
					return GL_FALSE;
				}
			}
			#else
			if(gGoldenImage[j*view_width + i] != gResultImage[j*view_width + i])
			{
				printf("======================================\n");
				printf(" <%d> CheckResult Fail!!!.\n", frame_count);
				printf(" <i:%d, j:%d>ref(0x%08x), out(0x%08x)\n", i, j, gGoldenImage[j*view_width + i], gResultImage[j*view_width + i]);
				printf("======================================\n");
				AppSetStatus(SLT_RES_ERR);
				return GL_FALSE;
			}
			#endif
		}	
		printf(" <%d> CheckResult Success.\n", frame_count);

		#if 0 //temp test
		if(5 == frame_count)
		{
			AppSetStatus(SLT_RES_OK);
		}
		#endif

		if(90 == frame_count)
		{
			AppSetStatus(SLT_RES_OK);
		}
		
		return GL_TRUE;
	}
	else
	{
		printf(" <%d> CheckResult skiped.\n", frame_count);
		return GL_FALSE;
	}	
}

///////////////////////////////////////////////////////////////////////////////
static GLboolean AppeglxCreate (GLint width, GLint height, GLint mode) 
{
    EGLConfig config;
	EGLConfig *configs = NULL;
	EGLNativeWindowType win;
	EGLint major, minor, num_config, max_num_config;
	int i;	
    EGLint attrib_list[] = { EGL_RED_SIZE, 8,  EGL_GREEN_SIZE, 8,  EGL_BLUE_SIZE, 8, 
			     EGL_ALPHA_SIZE, 8,  EGL_RENDERABLE_TYPE,  EGL_OPENGL_ES_BIT, 
			     EGL_SURFACE_TYPE,  EGL_WINDOW_BIT,  EGL_NONE };

	glesDisplay = eglGetDisplay( gEGLDispNative );
    if ( EGL_NO_DISPLAY == glesDisplay )
    {
		printf( "eglGetDisplay() failed (error 0x%x)\n", eglGetError() );
		return 1;
    }
    if ( EGL_FALSE == eglInitialize( glesDisplay, &major, &minor ) )
    {
		printf( "eglInitialize() failed (error 0x%x)\n", eglGetError() );
		return 1;
    }

	/* step2 - find the number of configs */
 	if ( EGL_FALSE == eglGetConfigs(glesDisplay, NULL, 0, &max_num_config) )
	{
		printf( "eglGetConfigs() failed to retrive the number of configs (error 0x%x)\n", eglGetError() );
		return 1;
	}

	if(max_num_config <= 0)
	{
		printf( "No EGLconfigs found\n" );
		return 1;
	}

	configs = (EGLConfig *)malloc( sizeof( EGLConfig) * max_num_config );
	if ( NULL == configs )
	{
		printf( "Out of memory\n" );
		return 1;
	}

    /* eglBindAPI( EGL_OPENGL_ES_API ); */
 	/* step3 - find a suitable config */
    if ( EGL_FALSE == eglChooseConfig( glesDisplay, attrib_list, configs, max_num_config, &num_config ) )
    {
		printf( "eglChooseConfig() failed (error 0x%x)\n", eglGetError() );
		return 1;
    }
    if ( 0 == num_config )
    {
		printf( "eglChooseConfig() was unable to find a suitable config\n" );
		return 1;
    }

	for ( i=0; i<num_config; i++ )
	{
		EGLint value;
		
		/*Use this to explicitly check that the EGL config has the expected color depths */
		eglGetConfigAttrib( glesDisplay, configs[i], EGL_RED_SIZE, &value );
		if ( 8 != value ) continue;
		eglGetConfigAttrib( glesDisplay, configs[i], EGL_GREEN_SIZE, &value );
		if ( 8 != value ) continue;
		eglGetConfigAttrib( glesDisplay, configs[i], EGL_BLUE_SIZE, &value );
		if ( 8 != value ) continue;
		eglGetConfigAttrib( glesDisplay, configs[i], EGL_ALPHA_SIZE, &value );
		if ( 8 != value ) continue;	
		config = configs[i];
		break;
	}

 	/* step4 - create a window surface (512x512 pixels) */
    win = (EGLNativeWindowType)hNativeWnd;
    glesSurface = eglCreateWindowSurface( glesDisplay, config, win, NULL );
    if ( EGL_NO_SURFACE == glesSurface )
    {
		printf( "eglCreateWindowSurface failed (error 0x%x\n", eglGetError() );
		return 1;
    }
 	/* step5 - create a context */
    glesContext = eglCreateContext( glesDisplay, config, EGL_NO_CONTEXT, NULL );
    if ( EGL_NO_CONTEXT == glesContext )
    {
		printf( "eglCreateContext failed (error 0x%x)\n", eglGetError() );
		return 1;
    }
 	/* step6 - make the context and surface current */
    if ( EGL_FALSE == eglMakeCurrent( glesDisplay, glesSurface, glesSurface, glesContext ) )
    {
		printf( "eglMakeCurrent failed (error 0x%x)\n", eglGetError() );
		return 1;
    }

	//glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearColor(0.3f, 0.7f, 0.4f, 1.0f);
	glShadeModel(GL_SMOOTH);  
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	return GL_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
static void AppeglxDelete() {
	
	glDeleteBuffers(3, nBufferName980);
	glDeleteBuffers(3, nBufferName9680);
	
	eglMakeCurrent (glesDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);		
	eglDestroyContext	(glesDisplay, glesContext);	
	eglDestroySurface	(glesDisplay, glesSurface);	
	eglTerminate (glesDisplay);
}

///////////////////////////////////////////////////////////////////////////////
static void AppeglxFlush() {	
	eglSwapBuffers(glesDisplay, glesSurface);
}

void InitOGLES(void)
{
	InitTestFramework();

	AppeglxCreate(view_width, view_height, 0);

	//glClearColorx(0,0,0,65536);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	
	glClearDepthx(65536); 

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glViewport(0, 0, view_width, view_height);

	// Set Projection 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glPerspectivef( 3.141592654f/4.0f, 1.0f, 1.0f, 1000.0f );	

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glGenTextures(1, &gTexID1); 	
	glBindTexture(GL_TEXTURE_2D , gTexID1 ); 
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
	glTexImage2D(GL_TEXTURE_2D, model.tl, model.ti, ( GLsizei )model.tw, ( GLsizei )model.th, 0, model.tf, model.tt, &DATA_globe2_0[0]);

	/*************Create VBO********************/
	//980 Sphere
	glGenBuffers(3,&nBufferName980[0]);

	glBindBuffer(GL_ARRAY_BUFFER,nBufferName980[VERTEX_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER,1677*sizeof(GLfixed),sphere_980pv,GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,nBufferName980[INDEX_BUFFER]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,2940*sizeof(GLshort),sphere_980pf,GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER,nBufferName980[TEXTURECOORD_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER,1118*sizeof(GLfixed),sphere_980pc,GL_STATIC_DRAW);

	//9680 Sphere
	glGenBuffers(3,&nBufferName9680[0]);

	glBindBuffer(GL_ARRAY_BUFFER,nBufferName9680[VERTEX_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER,15132*sizeof(GLfixed),sphere_9680pv,GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,nBufferName9680[INDEX_BUFFER]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,29040*sizeof(GLshort),sphere_9680pf,GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER,nBufferName9680[TEXTURECOORD_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER,10088*sizeof(GLfixed),sphere_9680pc,GL_STATIC_DRAW);
}

void DeInitOGLES(void)
{
	glDeleteTextures (1, &gTexID1);
	#ifdef TEST_FEATURE_DISPLAY_INFO
	if(m_pGLFont)
		SAFE_DELETE(m_pGLFont);
	#endif	
	AppeglxDelete();	
}

static unsigned int gRenderFrameCount;
static unsigned int time_interval_us = 0, curr_time = 0, last_time = 0, draw_poly_count = 0;
static float frames_per_sec = 0.f;
static float poly_per_sec = 0.f;
static float pixels_per_sec = 0.f;

///////////////////////////////////////////////////////////////////////////////
void Render(void)
{
	char text_fps[32];	
	char text_poly_cnt[32];	
	char text_polyps[32];	
	char text_object_cnt[32];	
	float font_scale_x = 1.2f;
	float font_scale_y = 1.5f;
	unsigned int test_start_time, test_end_time;
	test_start_time = (unsigned int)OS_GetTickCount();	
	gRenderTimeoutState = 0;

	//printf("GL_Draw start\n");

	GL_Draw();
	
#ifdef TEST_FEATURE_DISPLAY_INFO
	//fps =  framerate(100);
	//sprintf(text,"fps: %d",(int)fps);

	if(0 == gRenderFrameCount)
	{
		last_time = (unsigned int)OS_GetTickCount();
	}
	else if(TIME_CHECK_LOOP_CNT == gRenderFrameCount)
	{
		curr_time = (unsigned int)OS_GetTickCount();	
	}

	if(TIME_CHECK_LOOP_CNT == gRenderFrameCount)
	{
		unsigned polygon_cnt = 0;
		++gRenderFrameCount;
		time_interval_us = curr_time-last_time;
		frames_per_sec = ((float)TIME_CHECK_LOOP_CNT * 1000000.f)/(float)time_interval_us;
		polygon_cnt = draw_poly_count * TIME_CHECK_LOOP_CNT;
		poly_per_sec = (((float)polygon_cnt * 1000000.f)/(float)time_interval_us) * 2.7f/*temp*/;
		gRenderFrameCount = 0;
	}
	else
	{
		++gRenderFrameCount;
	}

	sprintf(text_fps,"frames/s  : %f", frames_per_sec);
	sprintf(text_poly_cnt,"poygon cnt: %d", draw_poly_count);
	sprintf(text_polyps,"M-poly/s  : %f", poly_per_sec/1000000.f);
	sprintf(text_object_cnt,"objects   : %d", gDrawEarthCnt);
	
	OrthoBegin();
	m_pGLFont->Draw2DStringLine(420*CONVERTFIXED,110*CONVERTFIXED,font_scale_x*CONVERTFIXED,font_scale_y*CONVERTFIXED,text_fps,GL_FALSE);
	m_pGLFont->Draw2DStringLine(420*CONVERTFIXED,90*CONVERTFIXED,font_scale_x*CONVERTFIXED,font_scale_y*CONVERTFIXED,text_poly_cnt,GL_FALSE);
	m_pGLFont->Draw2DStringLine(420*CONVERTFIXED,70*CONVERTFIXED,font_scale_x*CONVERTFIXED,font_scale_y*CONVERTFIXED,text_polyps,GL_FALSE);
	m_pGLFont->Draw2DStringLine(420*CONVERTFIXED,50*CONVERTFIXED,font_scale_x*CONVERTFIXED,font_scale_y*CONVERTFIXED,text_object_cnt,GL_FALSE);
	OrthoEnd();
#endif
	
	AppeglxFlush();

	test_end_time = (unsigned int)OS_GetTickCount();	 
	if(test_end_time >  test_start_time)
	{
		if((test_end_time - test_start_time) < 2000000)
		{
			gRenderTimeoutState = 0;//Success			
			//printf("GL_Draw end0(%d)\n", (test_end_time - test_start_time));
		}
		else
		{
			gRenderTimeoutState = 1;//Timeout			
		}
	}
	else
	{
		if((0xFFFFFFFFUL - test_start_time + test_end_time) < 2000000)
		{
			gRenderTimeoutState = 0;//Success
			//printf("GL_Draw end1(%d)\n", (0xFFFFFFFFUL - test_start_time + test_end_time));
		}
		else
		{
			gRenderTimeoutState = 1;//Timeout			
		}
	}
	
 }

static void SetPosition(int* x_offset, int* y_offset, int* z_offset, unsigned int drawEarthCnt)
{
	int offset_unit;
	float size_offset;
	size_offset = (drawEarthCnt / 14) * 1.5;
	drawEarthCnt = drawEarthCnt % 14;
	offset_unit = ((X1_0 * 1.5) + (X1_0 * size_offset));
	switch(drawEarthCnt)
	{
		case 0 : *x_offset = -offset_unit, *y_offset = offset_unit, *z_offset = offset_unit;
			break;
		case 1 : *x_offset = offset_unit, *y_offset = -offset_unit, *z_offset = -offset_unit;
			break;
		case 2 : *x_offset = offset_unit, *y_offset = -offset_unit, *z_offset = offset_unit;
			break;
		case 3 : *x_offset = -offset_unit, *y_offset = offset_unit, *z_offset = -offset_unit;
			break;
		case 4 : *x_offset = offset_unit, *y_offset = offset_unit, *z_offset = offset_unit;
			break;
		case 5 : *x_offset = -offset_unit, *y_offset = -offset_unit, *z_offset = -offset_unit;
			break;
		case 6 : *x_offset = -offset_unit, *y_offset = -offset_unit, *z_offset = offset_unit;
			break;
		case 7 : *x_offset = offset_unit, *y_offset = offset_unit, *z_offset = -offset_unit;
			break;

		case 8 : *x_offset = 0, *y_offset = 0, *z_offset = offset_unit*2;
			break;
		case 9 : *x_offset = 0, *y_offset = 0, *z_offset = -offset_unit*2;
			break;
		case 10 : *x_offset = 0, *y_offset = offset_unit*2, *z_offset = 0;
			break;
		case 11 : *x_offset = 0, *y_offset = -offset_unit*2, *z_offset = 0;
			break;
		case 12 : *x_offset = offset_unit*2, *y_offset = 0, *z_offset = 0;
			break;
		case 13 : *x_offset = -offset_unit*2, *y_offset = 0, *z_offset = 0;
			break;
		default:
			break;
	}
}


///////////////////////////////////////////////////////////////////////////////
//******* Main Render Loop *******************/
static void GL_Draw(void)
{
#if 1
	static float r=1.0f;
	static float g=1.0f;
	static float b=1.0f;
#else
	static float r=0.0f;
	static float g=0.0f;
	static float b=0.0f;
#endif
#if 0
    static float dr=0.003f;
    static float dg=0.002f;
    static float db=0.004f;
    r += dr;
    g += dg;
    b += db;    
    if( (dr<0 && r<0) || (dr>0 && r>1) ){ dr = -dr; }
    if( (dg<0 && g<0) || (dg>0 && g>1) ){ dg = -dg; }
    if( (db<0 && b<0) || (db>0 && b>1) ){ db = -db; }
#endif
    glColor4f( r,g,b,1.0f );

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer

	glPushMatrix();

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D , gTexID1 ); 

	draw_poly_count = 0;
	{	//9860*1

		//VBO Bind
		glBindBuffer(GL_ARRAY_BUFFER,nBufferName9680[VERTEX_BUFFER]);//Vertex buffer
		glVertexPointer(3, GL_FIXED, 0, 0);//vertex buffer Disable

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,nBufferName9680[INDEX_BUFFER]);//Index buffer

		glBindBuffer(GL_ARRAY_BUFFER,nBufferName9680[TEXTURECOORD_BUFFER]);//Texture coord buffer
		glTexCoordPointer(2, GL_FIXED, 0, 0);//Texture coord Pointer Disable
		//VBO Bind

		glTranslatex(X0_0,X0_0,g_depth/2);
		glRotatex(yrot,X0_0,X1_0,X0_0);
		glTranslatex(X0_0,X0_9,0);

		glDrawElements(/*drawMode*/GL_LINES, 9680 * 3, GL_UNSIGNED_SHORT, 0); 
		draw_poly_count += 9680; 	
	}
	glPopMatrix();

	glPushMatrix();
	if(vtxid==0)
	{		//980*10
		//VBO Bind
		glBindBuffer(GL_ARRAY_BUFFER,nBufferName980[VERTEX_BUFFER]);//Vertex buffer
		glVertexPointer(3, GL_FIXED, 0, 0);//vertex Pointer Disable

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,nBufferName980[INDEX_BUFFER]);//index buffer

		glBindBuffer(GL_ARRAY_BUFFER,nBufferName980[TEXTURECOORD_BUFFER]);//Texture Coord buffer
		glTexCoordPointer(2, GL_FIXED, 0, 0);//Texture coord Pointer Disable
		//VBO Bind

		glTranslatex(X0_0,X0_0,g_depth/2);
		glRotatex(xrot,X1_0,X0_0,X0_0);
		glRotatex(yrot,X0_0,X1_0,X0_0);
		glTranslatex(X0_0,X0_9,0);

		glScalex(X0_3,X0_3,X0_3);
		glTranslatex(X0_9 * 3 / 2, -X0_3, X0_0);	

		glPushMatrix();				
		glDrawElements(drawMode, 980 * 3, GL_UNSIGNED_SHORT, 0); 
		glPopMatrix();
		draw_poly_count += 980;

		glTranslatex(X0_0, -X0_9 * 3, X0_0);
		glRotatex(X1_0 * 72, X1_0, 0, 0);
		glTranslatex(X0_0, X0_9 * 3, X0_0);

		glDrawElements(drawMode, 980 * 3, GL_UNSIGNED_SHORT, 0); 
		draw_poly_count += 980;

		glTranslatex(X0_0, -X0_9 * 3, X0_0);
		glRotatex(X1_0 * 72, X1_0, 0, 0);
		glTranslatex(X0_0, X0_9 * 3, X0_0);
		glPushMatrix();
		glLoadIdentity();
		glPopMatrix();

		glDrawElements(drawMode, 980 * 3, GL_UNSIGNED_SHORT, 0); 
		draw_poly_count += 980;

		glTranslatex(X0_0, -X0_9 * 3, X0_0);
		glRotatex(X1_0 * 72, X1_0, 0, 0);
		glTranslatex(X0_0, X0_9 * 3, X0_0);
		glPushMatrix();
		glLoadIdentity();
		glPopMatrix();

		glDrawElements(drawMode, 980 * 3, GL_UNSIGNED_SHORT, 0); 
		draw_poly_count += 980;

		glTranslatex(X0_0, -X0_9 * 3, X0_0);
		glRotatex(X1_0 * 72, X1_0, 0, 0);
		glTranslatex(X0_0, X0_9 * 3, X0_0);
		glPushMatrix();
		glLoadIdentity();

		glPopMatrix();
		glDrawElements(drawMode, 980 * 3, GL_UNSIGNED_SHORT, 0); 
		draw_poly_count += 980;

		///// 6 ///////
		glTranslatex(X0_0, -X0_9 * 3, X0_0);
		glTranslatex(-X0_9 * 3, X0_0, X0_0);
		glRotatex(X1_0 * 72/2, X1_0, 0, 0);
		glTranslatex(X0_0, X0_9 * 3, X0_0);
		glPushMatrix();
		glLoadIdentity();
		glRotatex(xrot,X1_0,X0_0,X0_0);
		glRotatex(yrot,X0_0,X1_0,X0_0);
		glRotatex(yrot,X0_0,X0_0,-X1_0);
		glPopMatrix();		
		glDrawElements(drawMode, 980 * 3, GL_UNSIGNED_SHORT, 0); 
		draw_poly_count += 980;

		glTranslatex(X0_0, -X0_9 * 3, X0_0);
		glRotatex(X1_0 * 72, X1_0, 0, 0);
		glTranslatex(X0_0, X0_9 * 3, X0_0);
		glPushMatrix();
		glLoadIdentity();
		glRotatex(xrot,X1_0,X0_0,X0_0);
		glRotatex(yrot,X0_0,X1_0,X0_0);
		glRotatex(yrot,X0_0,X0_0,-X1_0);
		glPopMatrix();
		glDrawElements(drawMode, 980 * 3, GL_UNSIGNED_SHORT, 0);
		draw_poly_count += 980;

		glTranslatex(X0_0, -X0_9 * 3, X0_0);
		glRotatex(X1_0 * 72, X1_0, 0, 0);
		glTranslatex(X0_0, X0_9 * 3, X0_0);
		glPushMatrix();
		glLoadIdentity();
		glRotatex(xrot,X1_0,X0_0,X0_0);
		glRotatex(yrot,X0_0,X1_0,X0_0);
		glRotatex(yrot,X0_0,X0_0,-X1_0);
		glPopMatrix();
		glDrawElements(drawMode, 980 * 3, GL_UNSIGNED_SHORT, 0); 
		draw_poly_count += 980;

		glTranslatex(X0_0, -X0_9 * 3, X0_0);
		glRotatex(X1_0 * 72, X1_0, 0, 0);
		glTranslatex(X0_0, X0_9 * 3, X0_0);
		glPushMatrix();
		glLoadIdentity();
		glRotatex(xrot,X1_0,X0_0,X0_0);
		glRotatex(yrot,X0_0,X1_0,X0_0);
		glRotatex(yrot,X0_0,X0_0,-X1_0);
		glPopMatrix();
		glDrawElements(drawMode, 980 * 3, GL_UNSIGNED_SHORT, 0); 
		draw_poly_count += 980;

		glTranslatex(X0_0, -X0_9 * 3, X0_0);
		glRotatex(X1_0 * 72, X1_0, 0, 0);
		glTranslatex(X0_0, X0_9 * 3, X0_0);
		glPushMatrix();
		glLoadIdentity();
		glRotatex(xrot,X1_0,X0_0,X0_0);
		glRotatex(yrot,X0_0,X1_0,X0_0);
		glRotatex(yrot,X0_0,X0_0,-X1_0);
		glPopMatrix();
		glDrawElements(drawMode, 980 * 3, GL_UNSIGNED_SHORT, 0);
		draw_poly_count += 980;
	}
	else if(vtxid==1)
	{		
		//9680*10
		//VBO Bind
		glBindBuffer(GL_ARRAY_BUFFER,nBufferName9680[VERTEX_BUFFER]);//Vertex buffer
		glVertexPointer(3, GL_FIXED, 0, 0);//vertex Pointer Disable

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,nBufferName9680[INDEX_BUFFER]);//index buffer

		glBindBuffer(GL_ARRAY_BUFFER,nBufferName9680[TEXTURECOORD_BUFFER]);//Texture Coord buffer
		glTexCoordPointer(2, GL_FIXED, 0, 0);//Texture coord Pointer Disable
		//VBO Bind

		draw_poly_count = 0;
		#if 1
		glTranslatex(X0_0,X0_0,g_depth/2);
		glRotatex(xrot,X1_0,X0_0,X0_0);
		glRotatex(yrot,X0_0,X1_0,X0_0);
		//glTranslatex(X0_0,X0_9,0);
		glScalex(X0_1,X0_1,X0_1);
		
		for(int m = 0 ; m < gDrawEarthCnt ; m++)
		{
			int x_offset, y_offset, z_offset;
			glPushMatrix(); 			
			SetPosition(&x_offset, &y_offset, &z_offset, m);//(X1_0 * 3)
			glTranslatex(x_offset, y_offset, z_offset);
			glDrawElements(drawMode, 9680 * 3, GL_UNSIGNED_SHORT, 0); 
			glPopMatrix();
			
			draw_poly_count += 9680;
		}
		#else
		for(int m = 0 ; m < gRenderLoopCnt ; m++)
		{
			glTranslatex(X0_9 * 3 / 2, -X0_3, X0_0);	
			glPushMatrix();				
			glDrawElements(drawMode, 9680 * 3, GL_UNSIGNED_SHORT, 0); 
			glPopMatrix();
			draw_poly_count += 9680;

			glTranslatex(X0_0, -X0_9 * 3, X0_0);
			glRotatex(X1_0 * 72, X1_0, 0, 0);
			glTranslatex(X0_0, X0_9 * 3, X0_0);

			glDrawElements(drawMode, 9680 * 3, GL_UNSIGNED_SHORT, 0); 
			draw_poly_count += 9680;

			glTranslatex(X0_0, -X0_9 * 3, X0_0);
			glRotatex(X1_0 * 72, X1_0, 0, 0);
			glTranslatex(X0_0, X0_9 * 3, X0_0);
			glPushMatrix();
			glLoadIdentity();
			glPopMatrix();

			glDrawElements(drawMode, 9680 * 3, GL_UNSIGNED_SHORT, 0); 
			draw_poly_count += 9680;

			glTranslatex(X0_0, -X0_9 * 3, X0_0);
			glRotatex(X1_0 * 72, X1_0, 0, 0);
			glTranslatex(X0_0, X0_9 * 3, X0_0);
			glPushMatrix();
			glLoadIdentity();
			glPopMatrix();


			glDrawElements(drawMode, 9680 * 3, GL_UNSIGNED_SHORT, 0); 
			draw_poly_count += 9680;

			glTranslatex(X0_0, -X0_9 * 3, X0_0);
			glRotatex(X1_0 * 72, X1_0, 0, 0);
			glTranslatex(X0_0, X0_9 * 3, X0_0);
			glPushMatrix();
			glLoadIdentity();

			glPopMatrix();
			glDrawElements(drawMode, 9680 * 3, GL_UNSIGNED_SHORT, 0); 
			draw_poly_count += 9680;


			///// 6 ///////
			glTranslatex(X0_0, -X0_9 * 3, X0_0);
			glTranslatex(-X0_9 * 3, X0_0, X0_0);
			glRotatex(X1_0 * 72/2, X1_0, 0, 0);
			glTranslatex(X0_0, X0_9 * 3, X0_0);
			glPushMatrix();
			glLoadIdentity();
			glRotatex(xrot,X1_0,X0_0,X0_0);
			glRotatex(yrot,X0_0,X1_0,X0_0);
			glRotatex(yrot,X0_0,X0_0,-X1_0);
			glPopMatrix();		
			glDrawElements(drawMode, 9680 * 3, GL_UNSIGNED_SHORT, 0); 
			draw_poly_count += 9680;


			glTranslatex(X0_0, -X0_9 * 3, X0_0);
			glRotatex(X1_0 * 72, X1_0, 0, 0);
			glTranslatex(X0_0, X0_9 * 3, X0_0);
			glPushMatrix();
			glLoadIdentity();
			glRotatex(xrot,X1_0,X0_0,X0_0);
			glRotatex(yrot,X0_0,X1_0,X0_0);
			glRotatex(yrot,X0_0,X0_0,-X1_0);
			glPopMatrix();
			glDrawElements(drawMode, 9680 * 3, GL_UNSIGNED_SHORT, 0);
			draw_poly_count += 9680;

			glTranslatex(X0_0, -X0_9 * 3, X0_0);
			glRotatex(X1_0 * 72, X1_0, 0, 0);
			glTranslatex(X0_0, X0_9 * 3, X0_0);
			glPushMatrix();
			glLoadIdentity();
			glRotatex(xrot,X1_0,X0_0,X0_0);
			glRotatex(yrot,X0_0,X1_0,X0_0);
			glRotatex(yrot,X0_0,X0_0,-X1_0);
			glPopMatrix();
			glDrawElements(drawMode, 9680 * 3, GL_UNSIGNED_SHORT, 0); 
			draw_poly_count += 9680;


			glTranslatex(X0_0, -X0_9 * 3, X0_0);
			glRotatex(X1_0 * 72, X1_0, 0, 0);
			glTranslatex(X0_0, X0_9 * 3, X0_0);
			glPushMatrix();
			glLoadIdentity();
			glRotatex(xrot,X1_0,X0_0,X0_0);
			glRotatex(yrot,X0_0,X1_0,X0_0);
			glRotatex(yrot,X0_0,X0_0,-X1_0);
			glPopMatrix();
			glDrawElements(drawMode, 9680 * 3, GL_UNSIGNED_SHORT, 0); 
			draw_poly_count += 9680;

			glTranslatex(X0_0, -X0_9 * 3, X0_0);
			glRotatex(X1_0 * 72, X1_0, 0, 0);
			glTranslatex(X0_0, X0_9 * 3, X0_0);
			glPushMatrix();
			glLoadIdentity();
			glRotatex(xrot,X1_0,X0_0,X0_0);
			glRotatex(yrot,X0_0,X1_0,X0_0);
			glRotatex(yrot,X0_0,X0_0,-X1_0);
			glPopMatrix();
			glDrawElements(drawMode, 9680 * 3, GL_UNSIGNED_SHORT, 0);
			draw_poly_count += 9680;
		}
		#endif
	}
		
	glPopMatrix();

#if 0
	xrot+=xspeed*30;
	yrot+=yspeed*30;
#else
	xrot+=xspeed;
	yrot+=yspeed;
#endif

	//VBO Mode Disable
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 
	glBindBuffer(GL_ARRAY_BUFFER, 0); 		
}

///////////////////////////////////////////////////////////////////////////////
static void glPerspectivef(GLfloat fov, GLfloat aspect, GLfloat near_val, GLfloat far_val)
{
	GLfloat top = (GLfloat)(tan(fov*0.5) * near_val);
	GLfloat bottom = -top;
	GLfloat left = aspect * bottom;
	GLfloat right = aspect * top;

	glFrustumf(left, right, bottom, top, near_val, far_val);
}
//////////////////////////////////////////////////////////////////////////////////////

static void OrthoBegin(void)
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrthox(0,800*65536,0,480*65536,-65536,65536);	

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

static void OrthoEnd(void)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix(); 
}

/*************Calculate frame rate********/
static float framerate(int Poly)
{
    static float previous = 0;
    static int framecount = 0;
	static float finalfps = 0;
    framecount++;

    if ( framecount == 10 )
    {
        float time = (float)GetTime();
        float seconds = time - previous;
        float fps = framecount / seconds;
        previous = time;
		finalfps = fps;
        framecount = 0;
    }

	return finalfps;
}

/************* Texture ********/
static bool LoadTGA(TGAImage *texture, char *filename)				// Loads A TGA File Into Memory
{    
	GLubyte		TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};		// Uncompressed TGA Header
	GLubyte		TGAcompare[12];									// Used To Compare TGA Header
	GLubyte		header[6];										// First 6 Useful Bytes From The Header
	GLuint		bytesPerPixel;									// Holds Number Of Bytes Per Pixel Used In The TGA File
	GLuint		imageSize;										// Used To Store The Image Size When Setting Aside Ram
	GLuint		temp;											// Temporary Variable
	GLuint		type=GL_RGBA;									// Set The Default GL Mode To RBGA (32 BPP)

	OS_FILE file = OS_fopen(filename, "rb");							// Open The TGA File

	if(	file==NULL ||											// Does File Even Exist?
		OS_fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||	// Are There 12 Bytes To Read?
		memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0				||	// Does The Header Match What We Want?
		OS_fread(header,1,sizeof(header),file)!=sizeof(header))				// If So Read Next 6 Header Bytes
	{
		if (file == NULL)										// Did The File Even Exist? *Added Jim Strong*
			return GL_FALSE;										// Return False
		else													// Otherwise
		{
			OS_fclose(file);										// If Anything Failed, Close The File
			return GL_FALSE;										// Return False
		}
	}

	texture->width  = header[1] * 256 + header[0];				// Determine The TGA Width	(highbyte*256+lowbyte)
	texture->height = header[3] * 256 + header[2];				// Determine The TGA Height	(highbyte*256+lowbyte)
   
	if(	texture->width	<=0	||									// Is The Width Less Than Or Equal To Zero
		texture->height	<=0	||									// Is The Height Less Than Or Equal To Zero
		(header[4]!=24 && header[4]!=32))						// Is The TGA 24 or 32 Bit?
	{
		OS_fclose(file);											// If Anything Failed, Close The File
		return GL_FALSE;											// Return False
	}

	texture->bpp	= header[4];								// Grab The TGA's Bits Per Pixel (24 or 32)
	bytesPerPixel	= texture->bpp/8;							// Divide By 8 To Get The Bytes Per Pixel
	imageSize		= texture->width*texture->height*bytesPerPixel;	// Calculate The Memory Required For The TGA Data

	texture->imageData=(GLubyte *)malloc(imageSize);			// Reserve Memory To Hold The TGA Data

	if(	texture->imageData==NULL ||								// Does The Storage Memory Exist?
		OS_fread(texture->imageData, 1, imageSize, file)!=imageSize)	// Does The Image Size Match The Memory Reserved?
	{
		if(texture->imageData!=NULL)							// Was Image Data Loaded
			free(texture->imageData);							// If So, Release The Image Data

		OS_fclose(file);											// Close The File
		return GL_FALSE;											// Return False
	}

	for(GLuint i=0; i<int(imageSize); i+=bytesPerPixel)			// Loop Through The Image Data
	{															// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
		temp=texture->imageData[i];								// Temporarily Store The Value At Image Data 'i'
		texture->imageData[i] = texture->imageData[i + 2];		// Set The 1st Byte To The Value Of The 3rd Byte
		texture->imageData[i + 2] = temp;						// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
	}

	OS_fclose (file);												// Close The File


	glGenTextures(1, &texture->texID);							// Generate OpenGL texture IDs
	glBindTexture(GL_TEXTURE_2D, texture->texID);						// Bind Our Texture
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Linear Filtered
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// Linear Filtered
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	if (texture[0].bpp==24)										// Was The TGA 24 Bits
	{
		type=GL_RGB;											// If So Set The 'type' To GL_RGB
	}

	glTexImage2D(GL_TEXTURE_2D, 0, type, texture->width, texture->height, 0, type, GL_UNSIGNED_BYTE, texture->imageData);

	return GL_TRUE;												// Texture Building Went Ok, Return True
}

static double GetTime()
{
	return 0.001 * OS_GetTickCount();
}

//int OGLEarthSphere_VB()

