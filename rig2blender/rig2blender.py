import os
import sys
import math
import inspect

rigBounds = {}
restPose = {}

def processCommandLine( argv ):

   flagsOptions = {
      "-s": "modulePaths"
   }
   
   # We're going to get the entire blender command line parameters, but all
   #   we care about are the ones after "--". We need at least N commandline parameters
   try:
      index = argv.index("--") + 1
   except ValueError:
      index = 1
   argv = argv[index:]
   if len( argv ) < 1:
      print( "Usage: <input_json> -s <module_paths>" )
      exit( 0 )

   # Get the command line parameters
   returnValue = {
      "inputJson": argv[ 0 ],
      "modulePaths" : ""
   }
   counter = 1
   while ( len( argv ) > counter ):
      if argv[ counter ] in flagsOptions:
         try:
            # If this is a boolean flag, meaning the next argument is unrelated
            if returnValue[ flagsOptions[ argv[ counter ] ] ] == False:
               returnValue[ flagsOptions[ argv[ counter ] ] ] = True
               counter += 1
            else:
               returnValue[ flagsOptions[ argv[ counter ] ] ] = argv[ counter + 1 ]
               counter += 2
         except:
            counter += 1
            pass
      else:
         counter += 1

   return returnValue
   
def onError( rigId,
   errorCode,
   description ):

   print( rigId + " had error " + str(errorCode) + ": " + description )

# This is called per-character
def onUpdateBounds( rigId,
   startFrame,
   endFrame ):
   
   global globalStartFrame
   global globalEndFrame
   
   import bpy

   rigBounds[ rigId ] = (startFrame,endFrame)

def onFrame( rigId,
   frameNum,
   location,
   lengths,
   rotations,
   offsets ):
   
   import blenderRig
   import rig2pyBlender
   
   try:
      rigType = str(rig2pyBlender.getRigInfo( "", rigId, "type" ))
   except Exception as e:
      print( "Could not get type from rig2c: " + str(e) )
      return
   
   # Add a new skeleton for this rig, if necessary
   if rigId not in bpy.data.objects:
      if rigType == "humanoid":
         print( "Adding humanoid '" + rigId + "'" )
         blenderRig.addSkeleton( restPose, rigId, lengths, offsets )
      elif rigType == "solidObject":
         print( "Adding solid object '" + rigId + "'" )
         blenderRig.addSolidObject( rigId )         
      else:
         print( "Unknown rig type '" + rigType + "'" )
         return
      
      # Set key frames for when rigs are visible and hidden
      bounds = rigBounds[ rigId ]
      blenderRig.setRigVisibility( rigId, bounds[0], bounds[1] )

   # Pose the rig for this frame
   if rigType == "humanoid":
      blenderRig.poseSkeleton( rigId,
         frameNum,
         location,
         lengths,
         rotations,
         offsets )
   else:
      blenderRig.placeObject( rigId,
         frameNum,
         location )

if __name__ == "__main__":

   import bpy
   
   # Exit if this is an old version blender
   if (2, 80) > bpy.app.version:
      print( "This script requires blender >= 2.80" )
      exit()

   # Process the command line arguments
   args = processCommandLine( sys.argv )

   if args["modulePaths"] != "":
      paths = args["modulePaths"].split( ';' )
      for path in paths:
         sys.path.append( path )
   import rig2pyBlender
   import rig2pyHelper

   # Remove the default stuff
   try:
      bpy.data.objects.remove( bpy.data.objects['Camera'] )
   except:
      pass
   try:
      bpy.data.objects.remove( bpy.data.objects['Cube'] )
   except:
      pass
   try:
      bpy.data.objects.remove( bpy.data.objects['Light'] )
   except:
      pass

   # Set our units
   bpy.context.scene.unit_settings.system = "METRIC"

   # Set our callbacks
   rig2pyBlender.errorCallback( onError )
   rig2pyBlender.boundsCallback( onUpdateBounds )
   rig2pyBlender.frameCallback( onFrame )

   # Create our rest pose
   restPose = rig2pyHelper.createRestPose( "rig2pyBlender" )

   # Load all data from the file (will invoke the callback many times)
   rig2pyBlender.read( args["inputJson"] )
   
   # Set the FPS
   fps = "30"
   try:
      fps = rig2pyBlender.getInfo( "", "fps" )
   except:
      print( "Could not get info from rig2c" )
   bpy.context.scene.render.fps = float(fps)
   
   # Set the start/end frame
   globalStartFrame = 9999999
   globalEndFrame = 0
   for start,end in rigBounds.values():
      if start < globalStartFrame:
         globalStartFrame = start
      if end > globalEndFrame:
         globalEndFrame = end
        
   bpy.data.scenes[0].frame_start = globalStartFrame
   bpy.data.scenes[0].frame_end = globalEndFrame

   # Update the view
   bpy.data.scenes[0].frame_set( bpy.data.scenes[0].frame_start )
   bpy.context.view_layer.update()
