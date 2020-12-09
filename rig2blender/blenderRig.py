import bpy
import math
from mathutils import Vector, Quaternion, Matrix

# If true, adjusts to blender's coordinate system, where Z is up and Y is along the floor.
# This could become a parameter instead of hard-coding.
adjustForBlender = True

def addSkeleton( name,
   boneLengths,
   boneOffsets ):

   import rig2pyHelper
   
   # Create our rest pose, scaled by bone lengths if provided
   restPose = rig2pyHelper.createRestPose( "rig2pyBlender", boneLengths )

   # Create an armature and enter edit mode
   bpy.ops.object.armature_add( enter_editmode = True,
      location = [0,0,0] )

   # Create Object and Armature
   ob = bpy.context.object
   ob.name = name
   armature = ob.data
   armature.name = 'Armature'
   armature.show_axes = True
   armature.edit_bones.remove( armature.edit_bones[ 0 ] )

   for restPoseBone in restPose.values():
      
      # Create a new blender bone
      blenderBone = armature.edit_bones.new( restPoseBone.name )
      
      # If we are not the root joint
      if restPoseBone.parent != None:

         # Connect the joints
         blenderBone.parent = armature.edit_bones[ restPoseBone.parent.name ]
      
      # Set the head and tail
      blenderBone.head = restPoseBone.headAbs
      blenderBone.tail = restPoseBone.tailAbs

      # Special case: for some reason the ankle and toebase rolls are 180 degrees off in blender.
      # I'm guessing this is due to the rest pose being specified as a vector and not quaternion.
      if restPoseBone.name.lower().endswith("ankle") or restPoseBone.name.lower().endswith("toebase"):
            blenderBone.roll = math.pi
   
   # Back Into Object Mode
   bpy.ops.object.mode_set( mode='OBJECT' )
   
def addSolidObject( name ):
   
   # Create a sphere mesh
   bpy.ops.mesh.primitive_uv_sphere_add( location=[0,0,0], radius=0.1 )
   sphereMesh = bpy.context.active_object
   sphereMesh.name = name
   
   # Create a sphere material
   mat = bpy.data.materials.new( name='SphereMat' )
   alpha = 1 # 1 == opaque
   mat.diffuse_color = (.486,.176,.176, 1)
   
   # Assign the material to the mesh
   sphereMesh.data.materials.append( mat )
   
def setRigVisibility( rigId,
   startFrame,
   endFrame ):
   
   # Get the armature for this character
   rig = bpy.data.objects[ rigId ]
   
   # Set keyframes for showing the character ONLY within bounds
   rig.hide_viewport = True
   rig.keyframe_insert( data_path='hide_viewport', frame=-1 )
   rig.hide_viewport = False
   rig.keyframe_insert( data_path='hide_viewport', frame=startFrame )
   rig.hide_viewport = True
   rig.keyframe_insert( data_path='hide_viewport', frame=endFrame )

def poseSkeleton( rigId,
   frameNum,
   location,
   lengths,
   rotations,
   offsets ):

   import rig2pyHelper
   
   # Get the armature for this character
   skeleton = bpy.data.objects[ rigId ]

   # Add a keyframe for the pelvis and set it's location + rotation
   bone = skeleton.pose.bones["pelvis"]
   if adjustForBlender:
      bone.location = Vector((-location[0], location[2], location[1]))
   else:
      bone.location = Vector((location[0], location[1], location[2]))
   bone.keyframe_insert( data_path='location', frame=frameNum )

   for bone in skeleton.pose.bones:
   
      # Get the index of the quaternion in our data.
      index = rig2pyHelper.jointOrder.index( bone.name ) * 4
      
      # Get the rotation from our data and set it for this frame
      # Our data is [x,y,z,w] but Blender's mathutils wants [w,x,y,z]
      bone.rotation_quaternion = Quaternion(( rotations[ index+3 ], rotations[ index ], rotations[ index+1 ], rotations[ index+2 ] ))

      if adjustForBlender and bone.name == "pelvis":
         adjustment = Quaternion((0.0, 0.0, 1.0), math.pi) @ Quaternion((1.0, 0.0, 0.0), math.pi / 2)
         bone.rotation_quaternion = adjustment @ bone.rotation_quaternion
      
      # Insert this as a keyframe for this bone
      bone.keyframe_insert( data_path='rotation_quaternion', frame=frameNum )
      
      # Move to the next 4 rotation values
      index = index + 4

def placeObject( rigId,
   frameNum,
   location ):

   # Get the armature for this character
   object = bpy.data.objects[ rigId ]

   # Add a keyframe for the location
   if adjustForBlender:
      object.location = Vector((-location[0], location[2], location[1]))
   else:
      object.location = Vector((location[0], location[1], location[2]))
   object.keyframe_insert( data_path='location', frame=frameNum )