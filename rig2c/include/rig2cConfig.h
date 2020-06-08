#ifndef _RIG2C_API_CONFIG_
#define _RIG2C_API_CONFIG_

// All functions and types will have this prefix
#define API_PREFIX rig_

// We need two levels of indirection in order to apply the prefix properly
#define JOIN(a,b) JOIN_AGAIN(a,b)
#define JOIN_AGAIN(a,b) a ## b

// We need two levels of indirection in order to stringify properly
#define STRINGIFY(a) STRINGIFY_AGAIN(a)
#define STRINGIFY_AGAIN(a) #a

#if defined(RIG_API_EXPORTS)
	#if defined(_WIN32)

		#ifndef WIN32
			#define WIN32
		#endif

		#define NOMINMAX
		#include <windows.h>
		#define API_CALLING_CONVENTION __cdecl
		#define API_FUNC_DECLARE(a) __declspec( dllexport ) a API_CALLING_CONVENTION
		#define API_FUNC_NAME(a) JOIN(API_PREFIX,a)
		#define API_TYPE_NAME(a) JOIN(API_PREFIX,a)
		#define API_ARG1_TYPE
		#define API_ARG1_NAME
		#define API_ARG2_TYPE
		#define API_ARG2_NAME
		#define API_ARG_NAMES
		#define API_ARG_NAMES_WITH_COMMA
		#define API_ARG_NONE
		#define API_ARG_PREFIX

	#elif defined(__APPLE__)

		#include "TargetConditionals.h"

		#if (TARGET_IOS==1)

			#define API_CALLING_CONVENTION
			#define API_FUNC_DECLARE(a) a

		#elif (TARGET_OS_MAC==1)

			//#define API_CALLING_CONVENTION _System
			#define API_CALLING_CONVENTION
			#define API_FUNC_DECLARE(a) __attribute__((visibility("default"))) a API_CALLING_CONVENTION

		#endif

		#define API_FUNC_NAME(a) JOIN(API_PREFIX,a)
		#define API_TYPE_NAME(a) JOIN(API_PREFIX,a)
		#define API_ARG1_TYPE
		#define API_ARG1_NAME
		#define API_ARG2_TYPE
		#define API_ARG2_NAME
		#define API_ARG_NAMES
		#define API_ARG_NAMES_WITH_COMMA
		#define API_ARG_NONE
		#define API_ARG_PREFIX

	#elif defined(__ANDROID__)

		#ifndef ANDROID
			#define ANDROID
		#endif
		
		#include <jni.h>
		#include <android/native_window_jni.h>
			
		#if defined(RIG_API_JNI)

			#define API_CALLING_CONVENTION JNIEXPORT
			#define API_FUNC_DECLARE(a) API_CALLING_CONVENTION a JNICALL
			#define API_FUNC_NAME(a) java_com_intel_rig2c_interop_##a
			#define API_TYPE_NAME(a) a
			#define API_ARG1_TYPE JNIEnv *
			#define API_ARG1_NAME env		
			#define API_ARG2_TYPE jobject
			#define API_ARG2_NAME jobj
			#define API_ARG_NAMES env, jobj
			#define API_ARG_NAMES_WITH_COMMA env, jobj,
			#define API_ARG_NONE API_ARG1_TYPE API_ARG1_NAME, API_ARG2_TYPE API_ARG2_NAME
			#define API_ARG_PREFIX API_ARG_NONE,
			
		#else
      
			#define API_CALLING_CONVENTION
			#define API_FUNC_DECLARE(a) API_CALLING_CONVENTION a
			#define API_FUNC_NAME(a) JOIN(API_PREFIX,a)
			#define API_TYPE_NAME(a) JOIN(API_PREFIX,a)
			#define API_ARG1_TYPE
			#define API_ARG1_NAME
			#define API_ARG2_TYPE
			#define API_ARG2_NAME
			#define API_ARG_NAMES
			#define API_ARG_NAMES_WITH_COMMA
			#define API_ARG_NONE
			#define API_ARG_PREFIX
			
		#endif

	#else

		#define API_CALLING_CONVENTION
		#define API_FUNC_DECLARE(a) __attribute__((visibility("default"))) a API_CALLING_CONVENTION
		#define API_FUNC_NAME(a) JOIN(API_PREFIX,a)
		#define API_TYPE_NAME(a) JOIN(API_PREFIX,a)
		#define API_ARG1_TYPE
		#define API_ARG1_NAME
		#define API_ARG2_TYPE
		#define API_ARG2_NAME
		#define API_ARG_NAMES
		#define API_ARG_NAMES_WITH_COMMA
		#define API_ARG_NONE
		#define API_ARG_PREFIX

	#endif

#else

	#if defined(_WIN32)

		#ifndef WIN32
			#define WIN32
		#endif

		#define NOMINMAX
		#include <windows.h>
		#define API_CALLING_CONVENTION __cdecl
		#define API_FUNC_DECLARE(a) __declspec( dllimport ) a API_CALLING_CONVENTION
		#define API_FUNC_NAME(a) JOIN(API_PREFIX,a)
		#define API_TYPE_NAME(a) JOIN(API_PREFIX,a)
		#define API_ARG_NONE
		#define API_ARG_PREFIX

	#elif defined(__APPLE__)

		#include "TargetConditionals.h"

      #define API_FUNC_NAME(a) JOIN(API_PREFIX,a)
      #define API_TYPE_NAME(a) JOIN(API_PREFIX,a)
      #define API_ARG1_TYPE
      #define API_ARG1_NAME
      #define API_ARG2_TYPE
      #define API_ARG2_NAME
      #define API_ARG_NAMES
      #define API_ARG_NAMES_WITH_COMMA
      #define API_ARG_NONE
      #define API_ARG_PREFIX
      
		#if (TARGET_IOS==1)

			#define IOS
			#define API_CALLING_CONVENTION
			#define API_FUNC_DECLARE( a ) a

		#elif (TARGET_OS_MAC==1)

			#define MAC
			#define API_CALLING_CONVENTION
			#define API_FUNC_DECLARE(a) a

		#endif

	#elif defined(__ANDROID__)

		#ifndef ANDROID
			#define ANDROID
		#endif

		#if defined(RIG_API_JNI)

			#include <jni.h>

			#define API_CALLING_CONVENTION //JNIEXPORT
			#define API_FUNC_DECLARE( a ) API_CALLING_CONVENTION a JNICALL
			#define API_FUNC_NAME(a) JOIN(API_PREFIX,a)
			#define API_TYPE_NAME(a) JOIN(API_PREFIX,a)
         #define API_ARG1_TYPE
         #define API_ARG1_NAME
         #define API_ARG2_TYPE
         #define API_ARG2_NAME
         #define API_ARG_NAMES
         #define API_ARG_NAMES_WITH_COMMA
         #define API_ARG_NONE
			#define API_ARG_PREFIX JNIEnv * env,

		#else

			#define API_CALLING_CONVENTION
			#define API_FUNC_DECLARE( a ) a API_CALLING_CONVENTION
			#define API_FUNC_NAME(a) JOIN(API_PREFIX,a)
         #define API_TYPE_NAME(a) JOIN(API_PREFIX,a)
         #define API_ARG1_TYPE
         #define API_ARG1_NAME
         #define API_ARG2_TYPE
         #define API_ARG2_NAME
         #define API_ARG_NAMES
         #define API_ARG_NAMES_WITH_COMMA
         #define API_ARG_NONE
         #define API_ARG_PREFIX

		#endif

	#else

		#define API_CALLING_CONVENTION
		#define API_FUNC_DECLARE(a) a API_CALLING_CONVENTION
		#define API_FUNC_NAME(a) JOIN(API_PREFIX,a)
		#define API_TYPE_NAME(a) JOIN(API_PREFIX,a)
		#define API_ARG1_TYPE
		#define API_ARG1_NAME
		#define API_ARG2_TYPE
		#define API_ARG2_NAME
		#define API_ARG_NAMES
		#define API_ARG_NAMES_WITH_COMMA
		#define API_ARG_NONE
		#define API_ARG_PREFIX

	#endif

#endif

#endif
