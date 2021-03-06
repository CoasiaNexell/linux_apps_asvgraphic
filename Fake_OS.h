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
//	File       : Fake_OS.h
//	Description:
//	Author     : Yuni(yuni@mesdigital.com)
//	Export     :
//	History    :
//	   2007/01/22 Gamza first implementation
//------------------------------------------------------------------------------
#ifndef __FAKE_OS_H__
#define __FAKE_OS_H__

#include <EGL/egl.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//	Type definition
//------------------------------------------------------------------------------
#ifndef		NULL
	#define		NULL	0
#endif

#if defined( WIN32 )
typedef __int64 VR_LONG;
typedef unsigned __int64 VR_ULONG;
#else
typedef long long VR_LONG;
typedef unsigned long long VR_ULONG;
#endif

#if defined(linux)
	typedef	unsigned short	WORD;
	typedef	unsigned int	DWORD;

#ifndef true
	#define true	1
#endif

#ifndef	false
	#define false	0
#endif

#ifndef	byte
	typedef char	byte;
#endif
#else
	#include <windows.h>
#endif

#define DISP_WIDTH	800
#define DISP_HEIGHT	480


//------------------------------------------------------------------------------
//	Initialize platform
//------------------------------------------------------------------------------
void 	OS_InitFakeOS( void );

//------------------------------------------------------------------------------
//	Window
//------------------------------------------------------------------------------
typedef	void*	OS_Display;
typedef void*	OS_Window;
NativeWindowType 		OS_CreateWindow( void );
void OS_DestroyWindow( NativeWindowType window );

//------------------------------------------------------------------------------
//	Timer
//------------------------------------------------------------------------------
VR_ULONG OS_GetTickCount( void );
void OS_Sleep( unsigned int milliseconds );

//------------------------------------------------------------------------------
//	File I/O
//------------------------------------------------------------------------------
#define OS_SEEK_SET		0
#define OS_SEEK_CUR		1
#define OS_SEEK_END		2
typedef int OS_FILE;
OS_FILE OS_fopen( const char *filename, const char *mode );
int OS_fclose( OS_FILE stream );
int OS_fseek( OS_FILE stream, long offset, int origin );
unsigned int OS_fread( void *buffer, unsigned int size, unsigned int count, OS_FILE stream );

typedef unsigned int 		VR_BOOL;
#define VR_TRUE				1
#define VR_FALSE			0

VR_BOOL platform_system_init_display_device(void);
void platform_system_deinit_display_device(void);
EGLNativeDisplayType platform_system_get_native_display(void);
void platform_system_destory_native_display(void);


#ifdef __cplusplus
}
#endif
#endif // __FAKE_OS_H__

