# rig2blender
This tool takes rig files and imports them into [Blender](https://www.blender.org/). It does not create any cameras, meshes, etc. - it just creates armatures and poses them.
It uses the special module `rig2pyBlender`, which is syntactically identical to rig2py but is built against Blender's custom python distribution instead of the system's.

# Build
rig2Blender is a script so it doesn't need to be built; the rig2pyBlender module is built as part of the normal cmake build process.

Make sure Blender 2.8x is installed and operating properly. Blender 2.79 and below are not supported and will produce confusing errors if you try.

# Run
Assuming you are in the root directory of Rigging Tools, run the command below to open blender and import the rig data:

## Windows
`rig2blender/rig2blender.bat <rig file>`
## Everything else
`rig2blender/rig2blender.sh <rig file>`