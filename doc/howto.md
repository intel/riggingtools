# How-to
This is place to start if you haven't used Rigging Tools before. User stories are presented that mimick common tasks:

 - User story 1: [Visualizing with Blender](#user-story-1)
 - User story 2: [Scripting with python](#user-story-2)
 - User story 3: [Building an app with Unity](#user-story-2)

Use this workflow diagram when determining which tools are important to you. Start at the top and trace a path down to your target:
![Workflow](/img/workflow.png)

# User story 1
You are a data scientist or software developer with 3D keypoint data of people walking around a room.
You want to visualize the data from multiple perspectives and create a video you can share with others.
You also want to export the animation as an FBX file for other animators to use.

One way to accomplish this is to use an animation software such as Blender. Considering the workflow below, draw an imaginary path from the top down to Blender:

Referencing the workflow image, the path we need involves:
 - 3D keypoint data, let's assume as CSV files
 - kp2rig, which produces a rig as a JSON file
 - rig2blender, which consumes the rig in Blender

### Steps
|   |   |   |
| - | - | - |
| 1 | Prepare data | Conform your [3D keypoint data](/doc/kp2rigInputFiles.md). If your data is fundamentally different than what is supported, you may have to [develop your own keypoint implementation](/doc/kp2rig.md/#c++-classes) |
| 2 | Build the tools | Someday this may have a more robust release mechanism, but for now everything needs to be [built](/doc/build.md) |
| 3 | Create rig file | Run [kp2rig](/doc/kp2rig.md) to generate the rig file. This will create a file with name `seg_<start-frame>.json` |
| 4 | Import into Blender | Run [rig2blender](/doc/rig2blender.md) to open Blender and import the the rig file. |
| 5 | Create video/export FBX | Use Blender to create a video and export to FBX. Details regarding how to do this are outside the scope of this document |

# User story 2
You are a data scientist or software developer with 3D keypoint data of atheletes running on a soccer field.
You want to create a python script that will write out the location and velocity of each player at each frame in time.

Referencing the workflow image, the path we need involves:
 - 3D keypoint data, let's assume as CSV files
 - kp2rig, which produces a rig as a JSON file
 - rig2py, which helps read the rig in python

### Steps
|   |   |   |
| - | - | - |
| 1 | Prepare data | Conform your [3D keypoint data](/doc/kp2rigInputFiles.md). If your data is fundamentally different than what is supported, you may have to [develop your own keypoint implementation](/doc/kp2rig.md/#c++-classes) |
| 2 | Build the tools | Someday this may have a more robust release mechanism, but for now everything needs to be [built](/doc/build.md) |
| 3 | Create rig file | Run [kp2rig](/doc/kp2rig.md) to generate the rig file. This will create a file with name `seg_<start-frame>.json` |
| 4 | Create your python file | Reference the sample [worldCoordinates](/samples/worldCoordinates/README.md) for how to `import rig2py`, read the rig file, and handle callbacks |

# User story 3
You are an application developer or game developer with 3D keypoint data of a gymnist performing a routine.
You want to create a mobile application with Unity that allows a user to visualize and interact with the data.

Referencing the workflow image, the path we need involves:
 - 3D keypoint data, let's assume as CSV files
 - kp2rig, which produces a rig as a JSON file
 - rig2c, which helps read the rig in C
 - rig2cs, a C# wrapper for rig2c

 ### Steps
|   |   |   |
| - | - | - |
| 1 | Prepare data | Conform your [3D keypoint data](/doc/kp2rigInputFiles.md). If your data is fundamentally different than what is supported, you may have to [develop your own keypoint implementation](/doc/kp2rig.md/#c++-classes) |
| 2 | Build the tools | Someday this may have a more robust release mechanism, but for now everything needs to be [built](/doc/build.md). This does not need to be the same platform as the client's target |
| 3 | Create rig file | Run [kp2rig](/doc/kp2rig.md) to generate the rig file. This will create a file with name `seg_<start-frame>.json` |
| 4 | Build for target | Rigging Tools needs to be [built](/doc/build.md) for the client's target platform (Windows, iOS, etc.). This will create the platform-specific [rig2c](/doc/rig2c.md) required for the client application |
| 5 | Import the rig2c plugin | Import the platform specific bin/\<platform\>/rig2c.* into Unity as a [native plugin](https://docs.unity3d.com/Manual/PluginInspector.html):<ul><li>_Windows:_ `rig2c.dll`</li><li>_Mac:_ `rig2c.dylib`</li><li>_Linux,Android:_ `rig2c.so`</li></ul> |
| 6 | Import rig2cs scripts | Import the rig2cs scripts [librig2cInterop.cs](/rig2cs/librig2cInterop.cs) and [rig2c.cs](/rig2cs/rig2c.cs) into the Unity project |
| 7 | Create your scripts | Follow the example in [rig2csTest.cs](/rig2cs/test/rig2csTest.cs) for how to interact with rig2cs to read the rig file and handle callbacks |
