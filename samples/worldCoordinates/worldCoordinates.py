import os
import sys
import math

restPose = {}

def processCommandLine( argv ):

   flagsOptions = {
      "-s": "modulePaths",
      "-f": "frameNumber"
   }
   
   index = 1
   argv = argv[index:]
   if len( argv ) < 1:
      print( "Usage: <input_json> -s <module_paths>" )
      exit( 0 )

   # Get the command line parameters
   returnValue = {
      "inputJson": "",
      "modulePaths" : "",
      "frameNumber" : 0
   }
   counter = 0
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
         returnValue[ "inputJson" ] = argv[ counter ]
         counter += 1

   if returnValue[ "inputJson" ] == "":
      print( "Usage: <input_json> -s <module_paths>" )
      exit( 0 )

   return returnValue
   
def onError( rigId,
   errorCode,
   description ):
   
   print( rigId + " had error " + str(errorCode) + ": " + description )

def onFrame( rigId,
   frameNum,
   location,
   lengths,
   rotations,
   offsets ):

   import rig2pyHelper

   # Position and rotate the rest pose
   pose = rig2pyHelper.applyPose( restPose, location, lengths, rotations, offsets )
      
   # Pose now has all the bones in absolute space, where the bone's head is the value we want.
   # Put these XYZ values in an array with the same joint order as the rig
   absolutePositions = list("")
   for jointString in rig2pyHelper.jointOrder:
      position = pose[ jointString ].headAbs
      absolutePositions.append( str(position[0]) )
      absolutePositions.append( "," )
      absolutePositions.append( str(position[1]) )
      absolutePositions.append( "," )
      absolutePositions.append( str(position[2]) )
      absolutePositions.append( "," )
   absolutePositions.pop()
   absolutePositions.append( '\n' )

   oneFilename = rigId + ".txt"

   # It's a lot of data, so append to a CSV file.
   # TODO: make this more efficient
   file = open( oneFilename , 'a' )
   file.write( "".join( absolutePositions ) )
   file.close()

def main():

   # Process the command line arguments
   args = processCommandLine( sys.argv )

   if args["modulePaths"] != "":
      paths = args["modulePaths"].split( ';' )
      for path in paths:
         sys.path.append( path )

   import rig2py
   import rig2pyHelper
   import plot

   # Create a rest pose
   global restPose
   restPose = rig2pyHelper.createRestPose()
   print( "Joint order: " + str(rig2pyHelper.jointOrder) )

   # Set our callbacks
   rig2py.errorCallback( onError )
   rig2py.frameCallback( onFrame )

   # Load all data from the file (will invoke the callback many times)
   #try:
   #   rig2py.read( args["inputJson"] )
   #except RuntimeError as e:
   #   print( e )
   #   return

   # Plot all the txt files we created
   # TODO: use the frame number instead of assuming zero
   files = []
   for file in os.listdir("./"):
      if file.endswith(".txt"):
         files.append( file )
   plot.main( files )

if __name__ == "__main__":
   main()