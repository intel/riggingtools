"""
This file uses matplotlib to render a single frame from a CSV file.
The CSV file is just data where every line is a frame. Rig keypoints are assumed to be in the following order:

| Index | Joint |
| --    | -- |
|   0   | pelvis |
|   1   | rHIp |
|   2   | rKnee |
|   3   | rAnkle
|   4   | rToeBase |
|   5   | lHip |
|   6   | lKnee |
|   7   | lAnkle |
|   8   | lToeBase |
|   9   | spine2 |
|   10  | spine3 |
|   11  | spine4 |
|   12  | rShoulder |
|   13  | rElbow |
|   14  | rWrist |
|   15  | lShoulder |
|   16  | lElbow |
|   17  | lWrist |
|   18  | baseNeck |
|   19  | baseHead |

Example file:

  x1,y1,z1,x2,y2,z2,...xLastKP,yLastKP,zLastKP
  x1,y1,z1,x2,y2,z2,...xLastKP,yLastKP,zLastKP
  ...
"""

import sys
import numpy # pip3 install numpy
import matplotlib.pyplot as plt # pip3 install matplotlib

def showPlot( poseData ):

   # Convert the CSV string to a float array
   xyzData = [float(numeric_string) for numeric_string in poseData.split( "," )]

   # Create a new plot
   ax = plt.axes(projection="3d")

   # Separate the components
   xPoints = xyzData[ 0::3 ]
   yPoints = xyzData[ 1::3 ]
   zPoints = xyzData[ 2::3 ]

   # Normalize positions to the first joint
   xPoints = [ x - xPoints[0] for x in xPoints ]
   yPoints = [ y - yPoints[0] for y in yPoints ]
   zPoints = [ z - zPoints[0] for z in zPoints ]

   # Plot individual sections so the lines connect as expected
   plot( range(0,5),        ax, xPoints, yPoints, zPoints )
   plot( [0,5,6,7,8],       ax, xPoints, yPoints, zPoints )
   plot( [0,9,10,11,18,19], ax, xPoints, yPoints, zPoints )
   plot( [18,12,13,14],     ax, xPoints, yPoints, zPoints )
   plot( [18,15,16,17],     ax, xPoints, yPoints, zPoints )

   # Various plot options
   plt.axis('off')
   ax.set_xlim([-1,1])
   ax.set_ylim([-1,1])
   ax.set_zlim([-1,1])

   # Show the plot
   plt.show()

def plot( indices, ax, xPoints, yPoints, zPoints ):

   xPointsToPlot = []
   yPointsToPlot = []
   zPointsToPlot = []
   for i in indices:
      ax.text( xPoints[i], yPoints[i], zPoints[i], str(i) )
      xPointsToPlot.append( xPoints[i] )
      yPointsToPlot.append( yPoints[i] )
      zPointsToPlot.append( zPoints[i] )

   ax.plot( xPointsToPlot, 
      yPointsToPlot,
      zPointsToPlot,
      '-o' )

def main( filename ):
   
   # Read the first line
   file = open( filename , 'r' )
   line = file.readline().split( '\n' )[0]
   file.close()

   # Show the plot
   showPlot( line )
   
if __name__ == "__main__":

    # Just read a single filename
   argv = sys.argv[1:]
   if len( argv ) < 1:
      print( "Usage: <input_txt>" )
      exit( 0 )

   main( argv[0] )