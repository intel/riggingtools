# About
Reads a rig JSON file, traverses the rig hierarchy, and saves the world (global) joint coordinates to a CSV file. The format is {X,Y,Z} in units of meters.

Rigs store joints in a hierarchy as rotations relative to both their parent and the rest pose. This sample helps understand how to interpret the joint hierarchy and export rig joints as independent points-in-space. If no bone lengths or offsets are provided from the JSON file then default ones are used.

# Dependencies
 - python3 (3.5 or greater)
 - rig2py\[.so|.dll\](\<riggingtools\>/bin/\<platform\>/, usually built by cmake)
 - rig2pyHelper (<\riggingtools\>/rig2py/)
 - matplotlib (pip3 install matplotlib)
 - mathutils (pip3 install mathutils)

# Run
`python3 worldCoordinates.py <json file> -s "../../bin/<platform>/;../../rig2py/py"`

You can remove the `-s <paths separated by semicolons>` if you have rig2py\[.so|.dll\], rig2pyHelper.py, and plot.py in the same directory as this sample.


