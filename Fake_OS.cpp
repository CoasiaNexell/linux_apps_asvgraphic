//------------------------------------------------------------------------------
//
//	Copyright (C) 2003 MagicEyes Digital Co., Ltd All Rights Reserved
//	MagicEyes Digital Co. Proprietary & Confidential
//
//	MAGICEYES INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE AND
//	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
//
//	Module     :
//	File       : Fake_OS.cpp
//	Description:
//	Author     : Yuni(yuni@mesdigital.com)
//	Export     :
//	History    :
//	   2007/01/22 Gamza MP2530F port
//	   2007/01/08 Yuni  first implementation
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//	includes
//------------------------------------------------------------------------------
#include <Fake_OS.h>
#include <string.h>
#if defined(UNDER_CE)
	#error "todo...WinCE"
#elif defined(linux)
	#include <stdlib.h> /* for exit */
	#include <stdio.h> /* for exit */
	#include <unistd.h> /* for open/close .. */
	#include <fcntl.h> /* for O_RDWR */
	#include <sys/ioctl.h> /* for ioctl */
	#include <linux/fb.h> /* for fb_var_screeninfo, FBIOGET_VSCREENINFO */
	#include <sys/time.h> // for timeval, gettimeofday
#else
	#error "unknown platform"
#endif


//------------------------------------------------------------------------------
//  import
//------------------------------------------------------------------------------
//extern const unsigned char __door128_tga_Data[];
//extern const unsigned char __fire128_tga_Data[];
//extern const unsigned char 	__scene_gsd_Data[];
//extern const unsigned char 	__font_tga_Data[];
//extern const unsigned char 	__spot_raw_Data[];
//extern const unsigned char 	__plane_gsd_Data[];
//extern const unsigned char 	__font_raw_Data[];
//extern const unsigned char 	__floor_tga_Data[];
//extern const unsigned char 	__font_raw_Data[];
//extern const unsigned char 	__fire128_tga_Data[];
//extern const unsigned char 	__knot_gsd_Data[];
//extern const unsigned char  __alien1_gsd_Data       [];
//extern const unsigned char  __alien2_gsd_Data       [];
//extern const unsigned char  __alienShot_wav_Data    [];
//extern const unsigned char  __bala_gsd_Data         [];
//extern const unsigned char  __bunker_gsd_Data       [];
//extern const unsigned char  __bunker_tga_Data       [];
//extern const unsigned char  __bunkerXplosion_wav_Data[];
//extern const unsigned char  __creditos_tga_Data     [];
//extern const unsigned char  __espina_gsd_Data       [];
//extern const unsigned char  __font_raw_Data         [];
//extern const unsigned char  __keymap_tga_Data       [];
//extern const unsigned char  __nave1_tga_Data        [];
//extern const unsigned char  __nave2_tga_Data        [];
//extern const unsigned char  __portada_tga_Data      [];
//extern const unsigned char  __suelo_gsd_Data        [];
//extern const unsigned char  __suelo_tga_Data        [];
//extern const unsigned char  __tank_gsd_Data         [];
//extern const unsigned char  __tank_tga_Data         [];
//extern const unsigned char  __tankShot_wav_Data     [];
//extern const unsigned char  __tankXplosion_wav_Data [];
extern const unsigned char	__font_modify_Data		[];

const char* __FILE_LIST__[] =
{
//	"/c/resources/door128.tga",		(const char*)__door128_tga_Data,
//	"/c/resources/fire128.tga",		(const char*)__fire128_tga_Data,
//	"/c/resources/floor.tga", 		(const char*)__floor_tga_Data,
//	"/c/resources/scene.gsd",		(const char*)__scene_gsd_Data,
//	"/c/resources/font.tga", 		(const char*)__font_tga_Data,
//	"/c/resources/font.raw", 		(const char*)__font_raw_Data,
//	"/c/resources/knot.gsd", 		(const char*)__knot_gsd_Data,
//	"/c/resources/spot.raw"	, 		(const char*)__spot_raw_Data,
//	"/c/resources/plane.gsd",		(const char*)__plane_gsd_Data,
//	"/c/resources/font.raw"	, 		(const char*)__font_raw_Data,
//    "/c/resources/alien1.gsd"        , (const char*)__alien1_gsd_Data        ,
//    "/c/resources/alien2.gsd"        , (const char*)__alien2_gsd_Data        ,
//    "/c/resources/alienShot.wav"     , (const char*)__alienShot_wav_Data     ,
//    "/c/resources/bala.gsd"          , (const char*)__bala_gsd_Data          ,
//    "/c/resources/bunker.gsd"        , (const char*)__bunker_gsd_Data        ,
//    "/c/resources/bunker.tga"        , (const char*)__bunker_tga_Data        ,
//    "/c/resources/bunkerXplosion.wav", (const char*)__bunkerXplosion_wav_Data,
//    "/c/resources/creditos.tga"      , (const char*)__creditos_tga_Data      ,
//    "/c/resources/espina.gsd"        , (const char*)__espina_gsd_Data        ,
//    //"/c/resources/font.raw"          , (const char*)__font_raw_Data          ,
//    "/c/resources/keymap.tga"        , (const char*)__keymap_tga_Data        ,
//    "/c/resources/nave1.tga"         , (const char*)__nave1_tga_Data         ,
//    "/c/resources/nave2.tga"         , (const char*)__nave2_tga_Data         ,
//    "/c/resources/portada.tga"       , (const char*)__portada_tga_Data       ,
//    "/c/resources/suelo.gsd"         , (const char*)__suelo_gsd_Data         ,
//    "/c/resources/suelo.tga"         , (const char*)__suelo_tga_Data         ,
//    "/c/resources/tank.gsd"          , (const char*)__tank_gsd_Data          ,
//    "/c/resources/tank.tga"          , (const char*)__tank_tga_Data          ,
//    "/c/resources/tankShot.wav"      , (const char*)__tankShot_wav_Data      ,
//    "/c/resources/tankXplosion.wav"  , (const char*)__tankXplosion_wav_Data  ,
    "/c/resources/font_modify.tga"   , (const char*)__font_modify_Data  ,
	0
};

//------------------------------------------------------------------------------
//  export
//------------------------------------------------------------------------------
void 	OS_InitFakeOS( void )
{
    //ARMStartup_InitBoard();
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//	Window system
//------------------------------------------------------------------------------
#include <EGL/egl.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <linux/fb.h> /* for fb_var_screeninfo, FBIOGET_VSCREENINFO */
#include <unistd.h>		/* for open/close */
#include <fcntl.h>		/* for O_RDWR */
#include <sys/ioctl.h>	/* for ioctl */
#include <sys/types.h>	/* for mmap options */
#include <sys/mman.h>	/* for mmap options */

#include <xf86drm.h>
#include "dp.h"
#include "dp_common.h"
//#include "nexell_drmif.h"
#include <nx-drm-allocator.h>


#include <gbm.h>

#define NX_INFO				//printf
#define NX_ERR				printf

int g_drm_fd = -1;
static struct dp_device *g_drm_device;
struct gbm_device *g_gbm_dev;

static int platform_vmem_open_drm(void)
{
	int drm_fd = open_drm_device();
	if (drm_fd < 0) {
		#ifdef NEXELL_FEATURE_OPEN_RETRY_EN
		int i = 0;
		usleep(1000);
		for (i = 0 ; i < 5000 ; i++)
		{
			drm_fd = open_drm_device();
			if (drm_fd < 0)
			{
				if (i  && !(i % 100))
				{
					NX_INFO("[SYS] retry to open drm device.<%d>\n", i);
				}
				usleep(1000);
				continue;
			}
			else
			{
				NX_INFO("[SYS] opened drm device.<%d>\n", i);
				break;
			}
		}
		if (5000 == i)
		{
			NX_INFO("[SYS] failed to open drm device.<%d>\n", i);
			return -1;
		}
		#else /* org */
		NX_INFO("[SYS] failed to open drm device\n");
		return -1;
		#endif
	}
	NX_INFO("[SYS] drm_fd(0x%x)\n", drm_fd);
	return drm_fd;
}

VR_BOOL platform_system_init_display_device(void)
{
	int err;

	if (-1 == g_drm_fd)
	{
		g_drm_fd = platform_vmem_open_drm();
		if (g_drm_fd < 0) {
			NX_ERR("failed to open_drm_device\n");
			return VR_FALSE;
		}

		#if 1
		err = drmSetClientCap(g_drm_fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
		if (err < 0) {
			NX_ERR("drmSetClientCap() failed: %d %m\n", err);
			return VR_FALSE;
		}
		#endif

		g_drm_device = dp_device_open(g_drm_fd);
		if (!g_drm_device) {
			NX_ERR("fail : device open() %m\n");
			return VR_FALSE;
		}
	}
	return VR_TRUE;
}

void platform_system_deinit_display_device(void)
{
	if (g_drm_fd > -1)
	{
		if(g_gbm_dev)
		{
			gbm_device_destroy (g_gbm_dev);
			g_gbm_dev = NULL;
		}
		dp_device_close((struct dp_device *)g_drm_device);
		g_drm_device = NULL;
		drmClose(g_drm_fd);
		g_drm_fd = -1;
	}
}

int platform_system_get_display_fd(void)
{
	if (g_drm_fd < 0) {
		NX_ERR("failed to get drm fd\n");
		return -1;
	}
	return g_drm_fd;
}

void* platform_system_get_display_device(void)
{
	if (!g_drm_device) {
		NX_ERR("failed to get g_drm_device\n");
		return NULL;
	}
	return g_drm_device;
}

EGLNativeDisplayType platform_system_get_native_display(void)
{
	if (-1 == g_drm_fd)
	{
		NX_INFO("[SYS]fail to init drm device\n");
		return NULL;
	}
	if (g_gbm_dev){ return g_gbm_dev; }
	g_gbm_dev = gbm_create_device(g_drm_fd);
	NX_INFO("[SYS]get_native_display. gbm_dev(0x%x), drm_fd(0x%x)\n", (int)g_gbm_dev, (int)g_drm_fd);
	return (EGLNativeDisplayType)g_gbm_dev;
}

void platform_system_destory_native_display(void)
{
	if(g_gbm_dev)
	{
		gbm_device_destroy (g_gbm_dev);
		g_gbm_dev = NULL;
	}
}

//----------------------------------------------------------------------------------
EGLNativeWindowType platform_system_create_native_window(unsigned int width, unsigned int height)
{
	if (!g_gbm_dev)
	{	
		NX_ERR("failed to get g_gbm_dev\n");
		return (EGLNativeWindowType)NULL;
	}
	VR_BOOL is_target = VR_TRUE;
	int flag = (is_target)? (GBM_BO_USE_RENDERING | GBM_BO_USE_SCANOUT) : GBM_BO_USE_RENDERING;
	NX_INFO("[SYS] create native_window. gbm_dev(0x%x)\n", (int)g_gbm_dev);
	#ifdef NX_FEATURE_AVM_TARGET_YUV422_EN
	EGLNativeWindowType platform_win= (EGLNativeWindowType)gbm_surface_create(g_gbm_dev, width, height, GBM_FORMAT_YUYV, flag);
	#else
	EGLNativeWindowType platform_win= (EGLNativeWindowType)gbm_surface_create(g_gbm_dev, width, height, GBM_FORMAT_ARGB8888, flag);
	#endif
	NX_INFO("[SYS] created egl_native_window(%p). gbm_dev(0x%x)\n", platform_win, (int)g_gbm_dev);
	return (EGLNativeWindowType)platform_win;
}

void platform_system_destroy_native_window(EGLNativeWindowType win)
{
	gbm_surface_destroy((struct gbm_surface *)win);
}

//----------------------------------------------------------------------------------
EGLNativeWindowType OS_CreateWindow( void )
{
	EGLNativeWindowType egl_native_win;
#if 0
	fbdev_window *win_native;
    win_native = (fbdev_window *)malloc( sizeof( fbdev_window ) );
    memset(win_native, 0x0, sizeof( fbdev_window ));

	//set window size
    win_native->width = DISP_WIDTH;
    win_native->height = DISP_HEIGHT;
#else
	egl_native_win = platform_system_create_native_window(DISP_WIDTH, DISP_HEIGHT);
#endif
	return egl_native_win;
}

void OS_DestroyWindow( EGLNativeWindowType egl_native_win )
{
#if 0
	free(egl_native_win);
	egl_native_win = NULL;
#else
	platform_system_destroy_native_window(egl_native_win);
#endif
}

//------------------------------------------------------------------------------
VR_ULONG OS_GetTickCount( void )
{
#if defined(UNDER_CE)
	return GetTickCount();
#elif defined(linux)
	int result;
	struct timeval tod;

	result = gettimeofday(&tod, NULL);

	/* gettimeofday returns non-null on error*/
	if (0 != result) return 0;

	return ((VR_ULONG)tod.tv_sec) * 1000000ULL + tod.tv_usec;
#endif
}


//------------------------------------------------------------------------------
void OS_Sleep( unsigned int milliseconds )
{
#if defined(UNDER_CE)
	Sleep(milliseconds);
#elif defined(linux)
	usleep(milliseconds*1000);
#endif
}

//------------------------------------------------------------------------------
typedef struct
{
	const char*          m_pFileName;
	const unsigned char* m_pFileData;
	int                  m_CurPosition;
} NATIVE_FILE;
//------------------------------------------------------------------------------
OS_FILE OS_fopen( const char *filename, const char *mode )
{

	if( ('r' == mode[0]) || ('r' == mode[1]) )
	{
		int i = 0;

		while( __FILE_LIST__[i] )
		{
			if( !strcmp( filename, __FILE_LIST__[i] ) )
			{
				// create handle
				NATIVE_FILE* pFile = new NATIVE_FILE;

				pFile->m_pFileName = __FILE_LIST__[i];
				pFile->m_pFileData = (const unsigned char*)(__FILE_LIST__[i+1]);
				pFile->m_CurPosition = 0;

				return (OS_FILE)pFile;
			}
			else
				i+=2;
		}
		return 0;
	}
	else
		return 0;

	//return MES_FS_Open( filename, MES_O_RDONLY );

}

//------------------------------------------------------------------------------
int OS_fclose( OS_FILE stream )
{

	delete (NATIVE_FILE*)stream;
	return 0;

	//return MES_FS_Close( stream );
}

//------------------------------------------------------------------------------
int OS_fseek( OS_FILE stream, long offset, int origin )
{
	//return MES_FS_Seek( stream, offset, origin );

	int newPosition;

	// NATIVE_SEEK_CUR, NATIVE_SEEK_SET�� ����.
	switch( origin )
	{
	case OS_SEEK_CUR:
		newPosition = ((NATIVE_FILE*)stream)->m_CurPosition + offset;
		break;

	case OS_SEEK_SET:
		newPosition = offset;
		break;
	case OS_SEEK_END:
	default:
		return -1;
	}

	if( newPosition >= 0 )
	{
		((NATIVE_FILE*)stream)->m_CurPosition = newPosition;
		return 0;
	}
	else
		return -1;
}

//------------------------------------------------------------------------------
unsigned int OS_fread( void *buffer, unsigned int size, unsigned int count, OS_FILE stream )
{

	size_t copiedSize = size * count;
	memcpy( buffer, ((((NATIVE_FILE*)stream)->m_pFileData)+(((NATIVE_FILE*)stream)->m_CurPosition)), copiedSize );
	((NATIVE_FILE*)stream)->m_CurPosition += copiedSize;

	return copiedSize;

	//return MES_FS_Read( stream, buffer, size*count );
}

