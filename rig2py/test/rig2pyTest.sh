#!/bin/bash

SCRIPT_DIR="$( dirname $0 )"
IMPORTANT_PYTHON_FILE="$SCRIPT_DIR/rig2pyTest.py"

# We need to specify the path to the rig2py binary module
if [[ "$OSTYPE" == "linux-gnu" ]]; then
   MODULE_PATHS="$SCRIPT_DIR;$SCRIPT_DIR/../../bin/linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
   MODULE_PATHS="$SCRIPT_DIR;$SCRIPT_DIR/../../bin/macosx"
else
   MODULE_PATHS="$SCRIPT_DIR;$SCRIPT_DIR/../../bin/win64"
fi

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

# Start python
python3 $IMPORTANT_PYTHON_FILE -- $@ -s $MODULE_PATHS
