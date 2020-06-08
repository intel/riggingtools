using System;
using System.Runtime.InteropServices;

#if UNITY_5_3_OR_NEWER
   using AOT;
   using UnityEngine;
#endif

public class rig2cs : IDisposable
{
   public struct Position { public double x, y, z; }
   public struct Rotation { public double x, y, z, w; };
   [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
   private struct NativeJoint
   {
      public IntPtr name;
      public IntPtr parentName;
      public double offsetX;
      public double offsetY;
      public double offsetZ;
      public double quaternionX;
      public double quaternionY;
      public double quaternionZ;
      public double quaternionW;
      public double length;
      public int index;
   }

   public class Joint
   {
      public string name;
      public string parentName;
      public double[] offset;
      public double[] quaternion;
      public double length;
      public int index;
   }

   #region Public delegates and events

   public delegate void ErrorDelegate( string rigId,
      int errorCode,
      string description );
   public delegate void BoundsDelegate( string rigId,
      int startTimestamp,
      int endTimestamp );
   public delegate void FrameDelegate( string rigId,
      int timestamp,
      Position position,
      Rotation [] rotations,
      double [] boneLengths,
      Position [] boneOffsets );
      
   public static event ErrorDelegate Error;
   public static event BoundsDelegate Bounds;
   public static event FrameDelegate Frame;

   #endregion

   #region Internal delegates and events

#if UNITY_5_3_OR_NEWER
   [MonoPInvokeCallback(typeof(rig2cInterop.OnErrorDelegate))] 
#endif
   private static void InternalErrorHandler( string rigId,
      int errorCode,
      string description )
   {
      if ( Error != null )
         Error.Invoke( rigId, errorCode, description );
   }
   private rig2cInterop.OnErrorDelegate _internalErrorDelegate;

#if UNITY_5_3_OR_NEWER
   [MonoPInvokeCallback(typeof(rig2cInterop.OnErrorDelegate))] 
#endif
   private static void InternalBoundsHandler( string rigId,
      int startTimestamp,
      int endTimestamp )
   {
      if ( Bounds != null )
         Bounds.Invoke( rigId, startTimestamp, endTimestamp );
   }
   private rig2cInterop.OnBoundsDelegate _internalBoundsDelegate;

#if UNITY_5_3_OR_NEWER
   [MonoPInvokeCallback(typeof(rig2cInterop.OnErrorDelegate))] 
#endif
   private static void InternalFrameHandler( string rigId,
      int frameTimestamp,
      IntPtr nativeLocationXYZ,
      IntPtr nativeBoneRotations,
      int numBoneRotations,
      IntPtr nativeBoneLengths,
      int numBoneLengths,
      IntPtr nativeBoneOffsets,
      int numBoneOffsets )
   {
      // Copy the position to a managed struct
      Position position = (Position)Marshal.PtrToStructure( nativeLocationXYZ, typeof(Position) );

      // Copy the rotations to an array of managed structs
      Rotation [] rotations = new Rotation[ numBoneRotations ];
      for ( int i = 0; i < numBoneRotations; ++i )
      {
         rotations[i] = (Rotation)Marshal.PtrToStructure( IntPtr.Add(nativeBoneRotations, i*8*4), typeof(Rotation) );
      }
      
      // Copy the lengths to an array of doubles
      double [] lengths = new double[ numBoneLengths ];
      if ( numBoneLengths > 0 )
      {
         Marshal.Copy( nativeBoneLengths, lengths, 0, numBoneLengths );
      }

      // Copy the offsets to an array of managed structs
      Position [] offsets = new Position[ numBoneOffsets ];
      for ( int i = 0; i < numBoneOffsets; ++i )
      {
         offsets[i] = (Position)Marshal.PtrToStructure( IntPtr.Add(nativeBoneOffsets, i*8*3), typeof(Position) );
      }
      
      // Invoke the callback on this thread (meaning it will block until you're done with it!)
      if ( Frame != null )
         Frame.Invoke( rigId,
            frameTimestamp,
            position,
            rotations,
            lengths,
            offsets );
   }
   private rig2cInterop.OnFrameDelegate _internalFrameDelegate;

   #endregion

   #region Methods

   public void Initialize()
   {
      if ( IsInitialized )
         return;
         
      // Initialize
      try
      {
         if ( rig2cInterop.RETURN_CODE.NO_ERROR != rig2cInterop.rig_initialize( IntPtr.Zero ) )
         {
            throw new ApplicationException( "Failed loading library rig2c" );
         }
      }
      catch ( EntryPointNotFoundException e )
      {
         throw new EntryPointNotFoundException( "Could not load plugin rig2c", e );
      }

      // Make sure to keep persistent instances for internal delegates
      // in order to avoid the garbage collector destroying them.
      _internalErrorDelegate = new rig2cInterop.OnErrorDelegate(InternalErrorHandler);
      _internalBoundsDelegate = new rig2cInterop.OnBoundsDelegate(InternalBoundsHandler);
      _internalFrameDelegate = new rig2cInterop.OnFrameDelegate(InternalFrameHandler);
      
      // Register callbacks
      rig2cInterop.rig_setErrorCallback(_internalErrorDelegate);
      rig2cInterop.rig_setBoundsCallback(_internalBoundsDelegate);
      rig2cInterop.rig_setFrameCallback(_internalFrameDelegate);

   }
   public void Dispose()
   {
      // Uninitalize
      if (IsInitialized)
      {
         IsInitialized = false;
         rig2cInterop.rig_uninitialize();
         GC.SuppressFinalize(this);
      }
   }
   public string GetInfo(string key)
   {
      System.Text.StringBuilder value = new System.Text.StringBuilder(512);
      int bytes = rig2cInterop.rig_getInfo(key, value, value.Capacity);
      return bytes > 0 ? value.ToString() : null;
   }
   public string GetRigInfo( string rigId,
      string key )
   {
      System.Text.StringBuilder value = new System.Text.StringBuilder(512);
      int bytes = rig2cInterop.rig_getRigInfo( rigId, key, value, value.Capacity );
      return bytes>0 ? value.ToString() : null;
   }
   public void Start( string url )
   {
      rig2cInterop.rig_startRead( url );
      rig2cInterop.RETURN_CODE returnCode = rig2cInterop.rig_getLastError();
      if ( returnCode != rig2cInterop.RETURN_CODE.NO_ERROR )
         throw new ApplicationException( "Some error happened in rig2c, should probably report something more useful here ;)" );
   }
   public void Stop()
   {
      rig2cInterop.rig_stopRead();
   }
   static public Joint getRestPoseHumanoid(string previousName)
   {
      Joint returnValue = null;

      IntPtr nativeMemory = Marshal.AllocHGlobal(System.Runtime.InteropServices.Marshal.SizeOf(typeof(NativeJoint)));
      rig2cInterop.RETURN_CODE returnCode = rig2cInterop.rig_getRestPoseHumanoid(previousName, nativeMemory);

      if (returnCode == rig2cInterop.RETURN_CODE.NO_ERROR)
      {
         NativeJoint nativeJoint = (NativeJoint)Marshal.PtrToStructure(nativeMemory, typeof(NativeJoint));
         returnValue = new Joint();
         returnValue.name = Marshal.PtrToStringAnsi(nativeJoint.name);
         returnValue.parentName = Marshal.PtrToStringAnsi(nativeJoint.parentName);
         returnValue.offset = new double[]{ nativeJoint.offsetX, nativeJoint.offsetY, nativeJoint.offsetZ };
         returnValue.quaternion = new double[] { nativeJoint.quaternionX, nativeJoint.quaternionY, nativeJoint.quaternionZ, nativeJoint.quaternionW };
         returnValue.index = nativeJoint.index;
         returnValue.length = nativeJoint.length;
      }

      return returnValue;
   }

#endregion

   #region Properties

   public bool IsInitialized { get; private set; }
   
   #endregion
}
