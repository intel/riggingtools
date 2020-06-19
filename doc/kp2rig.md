# kp2rig

"Keypoint-to-rig" (kp2rig) is a terminal utility written in C++ that converts 3D keypoints into a rig useful for animation.
Supported platforms are Linux, macOS, and Windows.
This page will focus on the tool, you can go [here](generated-rigs.md) to learn more about the output rig.

## Dependencies
### Bundled dependencies
Many dependencies are bundled so you don't have to worry about installing things; that said, if an installed module is found it will favor that over the bundled version.

Bundled dependencies include:
 - [ZFP](https://computing.llnl.gov/projects/floating-point-compression)
 - [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page)
 - [JSON for modern C++](https://github.com/nlohmann/json)
 - [CLI11](https://github.com/CLIUtils/CLI11) 
 - `kp2rig/kpDescriptor.json`. This is a keypoint descriptor file that describes the keypoint serialization layout without requiring code changes. Copy to either the working directory or the executable directory.
 
### Intel Integrated Performance Primitives
kp2rig uses an IPP low-pass-filter to smooth noisy data. This _should_ work on both Intel and non-Intel processors.
You can also modify the code to replace the provided IPP filter with one of your choosing.

If you need to install IPP:
1. Navigate to Intel's webpage [here](https://www.intel.com/content/www/us/en/develop/tools/integrated-performance-primitives.html)
2. Click the `Stand-Alone` link
3. Fill out the form if it prompts you (not sure if there is a way around this)
4. Download the `Intel® Integrated Performance Primitives for <platform>` package. You don't need any of the other packages.
5. Install for your platform

If kp2rig complains about not finding ipp* dependencies, then:
- Linux: `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/intel/ipp/lib`. Modify the path if it's installed somewhere else
- Windows: copy all "dll" files from IPP to the working directory. The dll files are usually located at "C:\Program Files (x86)\IntelSWTools\compilers_and_libraries\windows\redist\intel64_win\ipp". Alternatively you can add this directory to the PATH environment variable.



## Example
Imagine you have a folder called `/home/me/3d_keypoints` containing text files. Each text file file is CSV formatted, represents a single character, and has 400 rows (which means 400 frames).
We want to create a standard rig that can be loaded in a Unity app on iOS using rig2c.

1.  We need to run kp2rig to convert the folder of text files to a standard rig. The fact that this rig will eventually be used on iOS is irrelevant because the rigs are portable JSON files, so we can generate rigs using kp2rig on macOS, Linux, or Windows.
2.  Make sure you have kp2rig. If you are [building](build.md) it yourself it will be in the `bin/<platform>` directory.
3.  Open a console. `cd` to the directory containing the kp2rig binary (`bin/<platform>`) and type `./kp2rig`. It should print out some information and exit nicely without any errors; if not, look at the dependency requirements above. We haven't created a rig yet but have verified the program is ready-to-go.
4.  Now let's create a rig. Type `./kp2rig /home/me/3d_keypoints -u 0.1 --left`. This should take a few seconds, print some stuff out, and exit. Here's what we just did:

|  |  |
| ------ | ------ |
| `/home/me/3d_keypoints` | We told kp2rig where our input was. In this example we passed in the folder name, but we could also have passed in each file one at a time, or used shell-expansion like `/home/me/3d_keypoints/*.txt`|
| `-u 0.1` | We told kp2rig how to get values converted to meters. In this example the 3d keypoints in the text files had units of decimeters, so multiplying by 0.1 gets values converted to meters. | 
| `--left` | We told kp2ig that we want the rig to be a _left-handed coordinate system_, which matches what Unity expects (every software is different FYI) | 

Here are additional options not used in this example. The best way to view these is to run `./kp2rig` without any arguments:
|  |  |
| ------ | ------ |
| `-o` | Set the output directory for the rig file (or rig file segments). Default is the working directory | 
| `-r` | Frames-per-second (fps). Default is 30 | 
| `--smooth <value>` | Specify the smoothing algorithm {`none`\|`lpf_ipp`}. Defaul is `lpf_ipp` |
| `--max-gap <value>` | Maximum gap, in seconds, of missing frames to interpolate. Gaps larger than this will not interpolate but instead copy/paste the previous frame, resulting in a "freeze". Default is `0.5` |
| -s | Read from STDIN instead of files. This is useful for live streaming |
| --segsize <value> | _WIP_ Segment duration in seconds. Default is `0`, meaning output a monolithic file |

5. If all goes well, you should have a new `seg_<start_timestamp>.json` file in your directory - this is your rig file
6. You can now use this rig file in many applications through [rig2c](rig2c.md)

## C++ classes
 - [Pose](../kp2rig/src/Pose.hpp): Interface representing a pose in time for a single object, often initialized with keypoints. Implementations include _KpPoseMpii_16_, _KpPoseMpii_20_, and _BallPose_.
 - [Rig](../common/Rig.hpp): Data class representing the standard output rig. All _Pose_ objects are required to generate a single _Rig_.
 - [RigPose](../kp2rig/src/RigPose.hpp): Wrapper (almost decorator) providing additional members and functions for the _Rig_ class.
 - [AnimatedRig](../kp2rig/src/AnimatedRig.hpp): Contains all frames (poses) for an object. Provides tools to smooth, fill in, and write data.
 - [Animation](../kp2rig/src/Animation.hpp): Analagous to a scene, this is the highest-level class containing all AnimatedRigs.

## Workflow
There are two threads of operation:
 - Parse (main)
 - Process and write
 
### Parse thread
![Parse thread diagram](/img/parseThread.svg)

### Process and Write thread
![Process thread diagram](/img/processThread.svg)
