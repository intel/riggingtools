import os
import sys
import math
import inspect

globalStartFrame = 9999999
globalEndFrame = 0
characters = {}

class Character:
   name = ""
   startFrame = 9999999
   endFrame = 0
   numFrames = 0

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

   # Set the global start/end frame, if necessary
   if startFrame < globalStartFrame:
      globalStartFrame = startFrame
   if endFrame > globalEndFrame:
      globalEndFrame = endFrame
      
   import rig2py
   
   rigType = ""
   try:
      rigType = rig2py.getRigInfo( "", rigId, "type" )
   except RuntimeError as e:
      print( e )
      return
   
   if rigType == None or rigType == "":
      print( "Unknown rig type '" + rigType + "'" )

def onFrame( rigId,
   frameNum,
   location,
   lengths,
   rotations,
   offsets ):
   
   character = Character()
   
   if rigId in characters:
      character = characters[ rigId ]
   else:
      character.name = rigId
      characters[rigId] = character
      
   character.numFrames = character.numFrames + 1
   
   if frameNum < character.startFrame:
      character.startFrame = frameNum
   if frameNum > character.endFrame:
      character.endFrame = frameNum
      
   # Test the arrays by accessing every element
   for i in range(0,3):
      value = location[i]
      
      if abs(value) > 1000000:
         print( "'" + rigId + "' frame " + str(frameTimestamp) +
            ", location: Unreasonable value " + value )
         return

   if rotations != None:
      for i in range(0, int(len(rotations)/4)):
         for j in range(0,4):
            value = rotations[i*4+j];
            
            if abs(value) > 1:
               print( "'" + rigId + "' frame " + str(frameTimestamp) +
                  ", rotation: Unreasonable value " + value )
               return
               
   if lengths != None:
      for i in range(0, len(lengths)):
         value = lengths[i]
         
         if value > 1000 or value <= 0.01:
            print( "'" + rigId + "' frame " + str(frameTimestamp) +
               ", length: Unreasonable value " + value )
            return
            
   if offsets != None:
      for i in range(0, int(len(offsets)/3)):
         for j in range(0, 3):
            value = offsets[i*3+j];
            
            if abs(value) > 100:
               print( "'" + rigId + "' frame " + str(frameTimestamp) +
               ", offset: Unreasonable value " + value )
               return

def main():

   # Process the command line arguments
   args = processCommandLine( sys.argv )

   if args["modulePaths"] != "":
      paths = args["modulePaths"].split( ';' )
      for path in paths:
         sys.path.append( path )
   import rig2py

   # Check the version
   version = ""
   try:
      version = rig2py.getInfo( args["inputJson"], "version" )
      print( "VERSION=" + version )
   except RuntimeError as e:
      print( e )
      return

   # Set our callbacks
   rig2py.errorCallback( onError )
   rig2py.boundsCallback( onUpdateBounds )
   rig2py.frameCallback( onFrame )

   # Walk the rest post hierarchy
   joint = rig2py.getRestPoseHumanoid( "" )
   while joint != None:
      name = joint.name
      parent = joint.parentName
      offset = joint.offset
      quat = joint.quaternion
      index = joint.index
      joint = rig2py.getRestPoseHumanoid( joint.name )

   # Load all data from the file (will invoke the callback many times)
   try:
      rig2py.read( args["inputJson"] )
   except RuntimeError as e:
      print( e )
      return
   
   # Get some info
   fps = ""
   try:
      fps = rig2py.getInfo( "", "fps" )
      print( "FPS=" + fps )
   except RuntimeError as e:
      print( e )
      return
   
   # Print a summary
   if len(characters):
      averageNumFramesPerCharacter = 0
      for name,character in characters.items():
         averageNumFramesPerCharacter = averageNumFramesPerCharacter + character.numFrames
      averageNumFramesPerCharacter = averageNumFramesPerCharacter / len(characters)
      print( "------------------------------------------------------" )
      print( "Approx " + str(round(averageNumFramesPerCharacter)) + " frames per character, " +
         str(len(characters)) + " characters, " +
         "bounds " + str(globalStartFrame) + "->" + str(globalEndFrame) )
   
if __name__ == "__main__":
   main()
