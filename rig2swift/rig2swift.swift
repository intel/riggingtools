public enum ReturnCode: Int {
   case
   NO_ERROR =                0,
	BAD_FILE_VERSION =       -1,
   BAD_PATH =               -2,
   NO_FILE_LOADED =         -3,
   BAD_FILE_DATA =          -4,
   API_NOT_INITIALIZED =    -5,
   NO_CALLBACK =            -6,
   NO_MORE_DATA =           -7,
   UNKNOWN_ERROR =          -12345
}

public class rig2swift {

   // Event delegates
   public typealias ErrorDelegate = ((String, ReturnCode, String)->())
   public typealias BoundsDelegate = ((String, Int, Int)->())
   public typealias FrameDelegate = ((String, Int, Array<Double>, Array<Double>?, Array<Double>?, Array<Double>? )->())
   
   // Event members
   public var OnError: ErrorDelegate?
   public var OnBounds: BoundsDelegate?
   public var OnFrame: FrameDelegate?
   
   public init() {
   }
   public func initialize() {
      rig_initialize( bridgeRetained( obj: self ) )
      
      rig_setErrorCallback( { (rigId_CStr, errorCode, description_CStr) in
      
         let _self : rig2swift = bridge( ptr: rig_getPlatformContext() )
         if _self.OnError != nil {
            let rigId = String(cString: rigId_CStr!)
            let description = String(cString: description_CStr!)
            _self.OnError!( rigId, ReturnCode(rawValue: Int(errorCode.rawValue))!, description )
         }
      })
      rig_setBoundsCallback( { (rigId_CStr, startFrame, endFrame) in

         let _self : rig2swift = bridge( ptr: rig_getPlatformContext() )
         if _self.OnBounds != nil {
            let rigId = String(cString: rigId_CStr!)
            _self.OnBounds!( rigId, Int(startFrame), Int(endFrame) )
         }
         
      })
      rig_setFrameCallback( { ( rigId_CStr,
         frameTimestamp,
         locationXYZ,
         boneRotations,
         numBoneRotations,
         boneLengths,
         numBoneLengths,
         boneOffsets,
         numBoneOffsets ) in
         
         let _self : rig2swift = bridge( ptr: rig_getPlatformContext() )
         if _self.OnFrame != nil {
            let rigId = String(cString: rigId_CStr!)
            let location = Array( UnsafeBufferPointer(start: locationXYZ, count: 3 ) )
            var rotations : Array<Double>? = nil
            var lengths : Array<Double>? = nil
            var offsets : Array<Double>? = nil
            
            // If we have data, COPY from native C to swift arrays
            if numBoneRotations > 0 {
               rotations = Array( UnsafeBufferPointer(start: boneRotations, count: Int(numBoneRotations) * 4) )
            }
            if numBoneLengths > 0 {
               lengths = Array( UnsafeBufferPointer(start: boneLengths, count: Int(numBoneLengths)) )
            }
            if numBoneOffsets > 0 {
               offsets = Array( UnsafeBufferPointer(start: boneOffsets, count: Int(numBoneOffsets) * 3) )
            }
            _self.OnFrame!( rigId,
               Int(frameTimestamp),
               location,
               rotations,
               lengths,
               offsets )
         }
      })
   }
   public func uninitialize() {
      rig_uninitialize();
   }
   public func getInfo( attribute: String ) -> String {
      let unsafeBuffer = UnsafeMutablePointer<Int8>.allocate(capacity: 512)
      var returnValue = ""
      if rig_getInfo( "", attribute, unsafeBuffer, 512 ) == rig_NO_ERROR {
         returnValue = String(cString: unsafeBuffer)
      }
      return returnValue
   }
   public func getRigInfo( rigId: String, attribute: String ) -> String {
      let unsafeBuffer = UnsafeMutablePointer<Int8>.allocate(capacity: 512)
      var returnValue = ""
      if rig_getRigInfo( "", rigId, attribute, unsafeBuffer, 512 ) == rig_NO_ERROR {
         returnValue = String(cString: unsafeBuffer)
      }
      return returnValue
   }
   public func read( url: String ) {
      rig_read( url )
   }
}

// Helper interop functions
func bridgeRetained<T : AnyObject>(obj : T) -> UnsafeMutableRawPointer {
   return UnsafeMutableRawPointer(Unmanaged.passRetained(obj).toOpaque())
}
func bridge<T : AnyObject>(ptr : UnsafeRawPointer) -> T {
   return Unmanaged<T>.fromOpaque(ptr).takeUnretainedValue()
}
