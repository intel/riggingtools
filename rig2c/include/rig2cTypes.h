#ifndef _RIG2C_API_TYPES_
#define _RIG2C_API_TYPES_

#include "rig2cConfig.h"

#if defined( __ANDROID__ ) && defined(RIG2C_API_JNI)

	/* Primitive types */
	typedef jint API_TYPE_NAME( INT );
	typedef jlong API_TYPE_NAME( UINT );
	typedef jlong API_TYPE_NAME( UINT64 );
	typedef jstring API_TYPE_NAME( STRING );
	typedef jobject API_TYPE_NAME( STRING_REF ); // StringBuilder only
	typedef jfloat API_TYPE_NAME( FLOAT );
	typedef jdouble API_TYPE_NAME( DOUBLE );
	typedef void API_TYPE_NAME( VOID );
	typedef jlong API_TYPE_NAME( VOID_PTR );
	typedef jbyteArray API_TYPE_NAME( BYTE_ARRAY );

#else

	typedef int API_TYPE_NAME( INT );
	typedef unsigned int API_TYPE_NAME( UINT );
	typedef unsigned long long API_TYPE_NAME( UINT64 );
	typedef const char * API_TYPE_NAME( STRING );
	typedef char * API_TYPE_NAME( STRING_REF );
	typedef float API_TYPE_NAME( FLOAT );
	typedef double API_TYPE_NAME( DOUBLE );
	typedef double * API_TYPE_NAME( DOUBLE_ARRAY );
	typedef const double * API_TYPE_NAME( CONST_DOUBLE_ARRAY );
	typedef void API_TYPE_NAME( VOID );
	typedef void * API_TYPE_NAME( VOID_PTR );
	typedef unsigned char * API_TYPE_NAME( BYTE_ARRAY );

#endif

/* This is required to avoid clashing with NO_ERROR defined on Windows platforms.
   If this causes a problem it can be commented out and our enum value renamed. */
#undef NO_ERROR

/* Defines a joint */
struct API_TYPE_NAME( JOINT )
{
   API_TYPE_NAME( STRING ) name;
   API_TYPE_NAME( STRING ) parentName;
   API_TYPE_NAME( DOUBLE ) offsetX;
   API_TYPE_NAME( DOUBLE ) offsetY;
   API_TYPE_NAME( DOUBLE ) offsetZ;
   API_TYPE_NAME( DOUBLE ) quaternionX;
   API_TYPE_NAME( DOUBLE ) quaternionY;
   API_TYPE_NAME( DOUBLE ) quaternionZ;
   API_TYPE_NAME( DOUBLE ) quaternionW;
   API_TYPE_NAME( DOUBLE ) length;
   API_TYPE_NAME( INT ) index;
};

#if defined( __ANDROID__ ) && defined(RIG2C_API_JNI)

   /* Primitive types */
   typedef jobject API_TYPE_NAME( JOINT_REF );
   
#else

   typedef struct API_TYPE_NAME( JOINT ) * API_TYPE_NAME( JOINT_REF );

#endif

/* Return codes */
typedef enum
{
   API_TYPE_NAME( NO_ERROR ) =                0,
	API_TYPE_NAME( BAD_FILE_VERSION ) =       -1,
   API_TYPE_NAME( BAD_PATH ) =               -2,
   API_TYPE_NAME( NO_FILE_LOADED ) =         -3,
   API_TYPE_NAME( BAD_FILE_DATA ) =          -4,
   API_TYPE_NAME( API_NOT_INITIALIZED ) =    -5,
   API_TYPE_NAME( NO_CALLBACK ) =            -6,
   API_TYPE_NAME( NO_MORE_DATA ) =           -7,
   API_TYPE_NAME( UNKNOWN_ERROR ) =          -12345
} API_TYPE_NAME( RETURN_CODE );

/* Callbacks */
typedef API_TYPE_NAME( VOID )( API_CALLING_CONVENTION *OnErrorDelegate )( API_TYPE_NAME( STRING ) rigId,
   API_TYPE_NAME( RETURN_CODE ),
   API_TYPE_NAME( STRING ) nullTerminatedDescription );
typedef API_TYPE_NAME( VOID )( API_CALLING_CONVENTION *OnBoundsDelegate )( API_TYPE_NAME( STRING ) rigId,
   API_TYPE_NAME( INT ) startTimestamp,
   API_TYPE_NAME( INT ) endTimestamp );
typedef API_TYPE_NAME( VOID )( API_CALLING_CONVENTION *OnFrameDelegate )( API_TYPE_NAME( STRING ) rigId,
   API_TYPE_NAME( INT ) frameTimestamp,
   API_TYPE_NAME( CONST_DOUBLE_ARRAY ) locationXYZ,
   API_TYPE_NAME( CONST_DOUBLE_ARRAY ) boneRotations,
   API_TYPE_NAME( INT ) numBoneRotations,
   API_TYPE_NAME( CONST_DOUBLE_ARRAY ) boneLengths,
   API_TYPE_NAME( INT ) numBoneLengths,
   API_TYPE_NAME( CONST_DOUBLE_ARRAY ) boneOffsets,
   API_TYPE_NAME( INT ) numBoneOffsets );

#endif
