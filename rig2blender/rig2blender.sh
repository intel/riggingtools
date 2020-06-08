#!/bin/bash
#
# If this script fails you may need to modify:
#   - BLENDER_ROOT
#   - BLENDER_VERSION_MAJOR
#
if [[ "$OSTYPE" == "linux-gnu" ]]; then # If LINUX
   BLENDER_ROOT="/usr/share/blender"
   BLENDER_CMD="$BLENDER_ROOT/blender"
elif [[ "$OSTYPE" == "darwin"* ]]; then # IF MAC
   BLENDER_ROOT="/Applications/Blender.app/Contents/Resources"
   BLENDER_CMD="$BLENDER_ROOT/../MacOS/Blender"
fi
BLENDER_VERSION_MAJOR=2

#
# The rest should be automatic so don't change anything below this line
#

printUsage()
{
   echo "Usage: $0 <input_json_file>"
}

# We require at least N arguments
if [ "$#" -lt "1" ]; then
   printUsage
   exit -1
fi

if [ ! -f "$1" ]; then
   echo "Input file not found"
   printUsage
   exit -1
fi

# Check for blender installation
BLENDER_VERSION=`find $BLENDER_ROOT -type d -name "$BLENDER_VERSION_MAJOR.*" -exec basename {} \;`
if [[ "$BLENDER_VERSION" == "" ]]; then
   echo "Could not find blender, please update BLENDER_ROOT in this script"
else
   echo "Found blender version $BLENDER_VERSION"
fi

# Verify the blender command
if [ -z "$BLENDER_CMD" ]; then
   echo "Failed to run the command '$BLENDER_CMD'"
   exit -1
fi

# Verify blender's python is installed
BLENDER_PYTHON_PATH="$BLENDER_ROOT/$BLENDER_VERSION/python"
blenderPythonCmd='command -v $BLENDER_PYTHON_PATH/bin/python*'
if [ -z "$blenderPythonCmd" ]; then
   echo "Failed to run Blender's special version of python. Please make sure blender installed python correctly"
   exit -1
fi

# Determine our bin/<platform> path for getting our module
if [[ "$OSTYPE" == "linux-gnu" ]]; then
   PLATFORM_STR="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
   PLATFORM_STR="macosx"
fi

# Copy our module to the blender addons directory IF it is newer
SCRIPT_DIR="$( dirname $0 )"
BLENDER_ADDONS_PATH="$BLENDER_ROOT/$BLENDER_VERSION/scripts/addons"
MODULE_NAME="rig2pyBlender.so"
echo "Copying $MODULE_NAME to blender's addons directory"
rsync -r -u $SCRIPT_DIR/../bin/$PLATFORM_STR/$MODULE_NAME $BLENDER_ADDONS_PATH/

# We need to specify two module paths:
#  1) The path to the blender python scripts
#  2) The path to the rig2pyHelper script
MODULE_PATHS="$SCRIPT_DIR;$SCRIPT_DIR/../rig2py/py"

# Start our python scripts FROM blender running in the background
IMPORTANT_PYTHON_FILE="$SCRIPT_DIR/rig2blender.py"
$BLENDER_CMD --python $IMPORTANT_PYTHON_FILE -- $@ -s $MODULE_PATHS
