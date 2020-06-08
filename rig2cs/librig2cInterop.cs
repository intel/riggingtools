using System;
using System.Runtime.InteropServices;

public static class rig2cInterop
{
#if UNITY_IOS && !UNITY_EDITOR
   const string LIB_NAME = "__Internal";
#else
   const string LIB_NAME = "rig2c";
#endif

   public enum RETURN_CODE
   {
      NO_ERROR = 0,
      BAD_FILE_VERSION = -1,
      BAD_PATH = -2,
      NOT_STARTED = -4,
      BAD_JSON = -5,
      BAD_HANDLE = -6,
      BAD_KEY = -7,
      BAD_DATA = -8,
      API_NOT_INITIALIZED = -9,
      NO_CALLBACK = -10,
      UNKNOWN_ERROR = -12345
   }

   /// <summary>
   /// Delegates for native callbacks
   /// </summary>
   [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
   public delegate void OnErrorDelegate( string rigId,
      int errorCode,
      string description );
      [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
   public delegate void OnBoundsDelegate( string rigId,
      int startTimestamp,
      int endTimestamp );
   [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
   public delegate void OnFrameDelegate( string rigId,
      int frameTimestamp,
      IntPtr locationXYZ,
      IntPtr boneRotations,
      int numBoneRotations,
      IntPtr boneLengths,
      int numBoneLengths,
      IntPtr boneOffsets,
      int numBoneOffsets );
   
   /// <summary>
   /// Native functions
   /// </summary>
   [DllImport(LIB_NAME)]
   public static extern RETURN_CODE rig_initialize( IntPtr platformContext );
   
   [DllImport(LIB_NAME)]
   public static extern void rig_uninitialize();
   
   [DllImport(LIB_NAME)]
   public static extern void rig_setErrorCallback( OnErrorDelegate functionPtr );
   
   [DllImport(LIB_NAME)]
   public static extern void rig_setBoundsCallback( OnBoundsDelegate functionPtr );
   
   [DllImport(LIB_NAME)]
   public static extern void rig_setFrameCallback( OnFrameDelegate functionPtr );
   
   [DllImport(LIB_NAME)]
   public static extern RETURN_CODE rig_getLastError();
   [DllImport(LIB_NAME)]
   public static extern RETURN_CODE rig_getRestPoseHumanoid( string previousName,
      IntPtr joint );

   [DllImport(LIB_NAME)]
   public static extern int rig_getInfo( string key,
      System.Text.StringBuilder value,
      int valueSize );

   [DllImport(LIB_NAME)]
   public static extern int rig_getRigInfo(string rigId,
      string key,
      System.Text.StringBuilder value,
      int valueSize);

   [DllImport(LIB_NAME)]
   public static extern void rig_startRead( string url );
   
   [DllImport(LIB_NAME)]
   public static extern void rig_stopRead();
}
