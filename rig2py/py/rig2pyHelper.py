"""
This file provides utilities that mimick what animation softwares do:
 - Creating a compatible rest pose
 - Posing characters
"""
import os
import sys
import math
import copy
from mathutils import Vector, Quaternion # pip3 install mathutils

# Define a global rest pose and joint order.
# We'll fill these in only once
jointOrder = []

# Define special joints
rootName = "pelvis"
connectToHead = [ "rHip", "lHip" ]
haveOffsets = [ "rHip", "lHip", "rShoulder", "lShoulder" ]

# Create a simple bone class to represent our rest pose
class Bone:
   def __init__(self, name):
      self.name = name
      self.headAbs = Vector(( 0., 0., 0. ))
      self.tailAbs = Vector(( 0., 0., 0. ))
      self.offset = Vector(( 0., 0., 0. ))
      self.rotation = Quaternion(( 1., 0., 0., 0. ))
      self.rotationAbs = Quaternion(( 1., 0., 0., 0. ))
      self.parent = None
      self.children = []
   def __repr__(self):
      return self.name + " head:" + str(self.headAbs) + ", tail:" + str(self.tailAbs)

def traverseAndUpdate( refDictionary, rootName, rotateOffsets = False ):  
   """
   Recursive function that updates the humanoid hierarchy with absolute values

   @refDictionary is a <str,Bone> dictionary of connected bones with relative values set, specifically:
      - Root bone must have bone.headAbs set to the absolute location of the rig, other bones should set bone.head to zero.
      - Every bone must have bone.tail set such that `bone.tail - bone.head` produces a vector with correct bone length.
        The orientation doesn't matter, so you can just set one component to the length and the others to zero.
      - Every bone must have bone.rotation set, where bone.rotation is a _relative_ rotation to the parent
      - Every bone must have bone.offset set
      - Every bone must have the connections bone.parent and bone.children set to other bones in refDictionary as appropriate
   @rootName is the dictionary key for the root bone
   @rotateOffsets is a boolean indicating whether or not offsets need to be rotated. This should be true if you didn't
   update the offsets from the frame callback

   This function updates refDictionary inplace and does not return any value.
   For each Bone in refDictionary, this function sets:
      - headAbs
      - tailAbs
      - rotationAbs
   """
   upVector = Vector(( 0., 1., 0. ))

   # Get the node to start with
   thisBone = refDictionary[ rootName ]

   # Get the length before we change heads and tails
   norm = (thisBone.tailAbs - thisBone.headAbs).magnitude

   # If we are not the root joint
   if thisBone.parent != None:

      # Set the absolute rotation for this bone (still relative to the rest pose though)
      thisBone.rotationAbs = thisBone.parent.rotationAbs @ thisBone.rotation

      if rotateOffsets == True:
         # Rotate using the parent's rotation:
         thisBone.offset.rotate( thisBone.parent.rotationAbs )

      # Set the head to the tail (or head) of the parent + any offset.
      if connectToHead.count( thisBone.name ) > 0:
         thisBone.headAbs = thisBone.parent.headAbs + thisBone.offset
      else:
         thisBone.headAbs = thisBone.parent.tailAbs + thisBone.offset

   else:

      # Assume head has already been set
      thisBone.rotationAbs = thisBone.rotation

   # Create an vector of the correct bone length   
   poseVec = upVector * norm

   # Rotate this vector using this bone's absolute rotation
   poseVec.rotate( thisBone.rotationAbs )
   
   # Set the tail
   thisBone.tailAbs = thisBone.headAbs + poseVec

   # For each child of this node
   for childBone in thisBone.children:
      traverseAndUpdate( refDictionary, childBone.name )

def createRestPose( moduleName = "rig2py",
   boneLengths = None,
   boneOffsets = None ):
   """
   Returns a <str,Bone> dictionary of Bone objects that represent the rest pose. Calls the rig2py API.
   Usually you only call this once to create a generic rest pose, then scale and pose for each character as needed.
   @moduleName is an optional name of the python module. This is useful for custom modules such as the one provided for Blender.
   @boneLengths is an optional array of bone lengths
   @boneOffsets is an optional array of bone offsets (shoulders, hips)

   @returns a <str,Bone> dictionary
   """
   rig2py = __import__( moduleName )

   returnValue = {}

   # Get each joint in the rest-post hierarchy, defined by the rig API,
   # creating associated bones as we do so
   joint = rig2py.getRestPoseHumanoid( "" )
   boneLengthIndex = 0
   while joint != None:

      # Create a new bone.
      newBone = Bone( joint.name )

      # Set the head offset, applying the bone offset if provided
      if boneOffsets != None:
         offsetMap = [ "rHip", "lHip", "rShoulder", "lShoulder" ]
         try:
            index = offsetMap.index( joint.name )
            newBone.offset = Vector(( boneOffsets[index * 3 + 0], boneOffsets[index * 3 + 1], boneOffsets[index * 3 + 2] ))
         except ValueError:
            newBone.offset = Vector(( joint.offset[0], joint.offset[1], joint.offset[2] ))
      else:
         newBone.offset = Vector(( joint.offset[0], joint.offset[1], joint.offset[2] ))

      # Set the bone's rotation
      newBone.rotation = Quaternion(( joint.quaternion[3], joint.quaternion[0], joint.quaternion[1], joint.quaternion[2] ))

      # Set the tail, applying the bone length if provided
      if boneLengths != None and boneLengthIndex < len(boneLengths):
         newBone.tailAbs = Vector(( 0., boneLengths[boneLengthIndex], 0. ))
      else:
         newBone.tailAbs = Vector(( 0., joint.length, 0. ))

      # If we are not the root joint
      if joint.parentName != "":

         # Connect
         newBone.parent = returnValue[ joint.parentName ]
         newBone.parent.children.append( newBone )
      
      # Add the bone to our return value
      returnValue[ joint.name ] = newBone

      # Keep track of the joint order, we'll need this later for interpreting the rotation data
      jointOrder.append( joint.name )

      # Get the next joint
      joint = rig2py.getRestPoseHumanoid( newBone.name )

      boneLengthIndex = boneLengthIndex + 1

   # Update the hierarchy
   traverseAndUpdate( returnValue, rootName )
   
   return returnValue

def applyPose( restPose, location, lengths, rotations, offsets ):
   """
   Returns a <str,Bone> dictionary that represent the pose.

   @restPose a <str,Bone> dictionary representing the rest pose returned from createRestPose   
   @location array of 3 floats representing x,y,z of the root joint   
   @lengths array of n floats representing bone lengths, where n is the number of joints
   @rotations array of n*4 floats representing quaternion components, where n is the number of joints
   @offsets array of m*3 floats, where m is the number of offsets

   @returns a <str,Bone> dictionary
   """

   # Start with a copy of the rest pose
   returnValue = copy.deepcopy( restPose )

   # Set the pose rotations we received
   for bone in returnValue.values():

      # Set the absolute rotation for this bone (still relative to the rest pose though)
      index = jointOrder.index( bone.name )
      i = index * 4
      bone.rotation = bone.rotation @ Quaternion(( rotations[i+3], rotations[i+0], rotations[i+1], rotations[i+2] ))

      # Overwrite the default bone lengths with the actual bone length
      if lengths != None and len(lengths) >= index:
         bone.tail = Vector( [0., lengths[index], 0.] )

      # Overwrite the default offsets with the actual offsets
      if offsets != None and haveOffsets.count( bone.name ) > 0:
         i = haveOffsets.index( bone.name ) * 3
         bone.offset = Vector(( offsets[i+0], offsets[i+1], offsets[i+2] ))

   # Set the root location and update the tail
   returnValue[ rootName ].headAbs = Vector(( location[0], location[1], location[2] ))
   returnValue[ rootName ].tailAbs = returnValue[ rootName ].headAbs + returnValue[ rootName ].tailAbs

   # Update the hierarchy
   traverseAndUpdate( returnValue, rootName )

   return returnValue