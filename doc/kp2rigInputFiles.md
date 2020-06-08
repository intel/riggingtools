# kp2rig input files

## 3D Keypoints

kp2rig only converts 3D keypoints (not 2D keypoints, maybe someday) to a standard rig. 3D keypoints are defined as 3 decimal values in `X,Y,Z` order.

Keypoint data is laid out according to the [kpDescriptor.json](#kpDescriptorjson) file, which is required to run kp2rig.
Using a JSON file allows the layout to be modified without the need to customize code as long as the same number and type of joints are used.
If you need a unique number of joints or different joint types then you will need to both modify the kpDescriptor.json file and create a new [KpImporter](/kp2rig/src/KpImporter.hpp).
 
kp2rig imports the following keypoint formats:
 - [CSV files/pipe](#csv)
 - [JSON files](#json)
 
## CSV
Keypoints can be input to kp2rig as a collection of CSV files, a single CSV file, or over stdin via the pipe operator (unix only).
Since CSV has no mechanism for defining schemas, the format is rigid and a bit redundant.
File-headers are not allowed, meaning every row must contain all the metadata it needs.

Each row in the CSV file/stdin stream must be formatted as follows:
 - FIELD 1 is *keypoint type*. Must match an entry in ['kpDescriptor.json'](#kpDescriptorjson)
 - FIELD 2 is *unique ID* of the character. Can be anything: human1, ball, John, etc.
 - FIELD 3 is *custom data*. This can be anything you want as long as there are no commas or colons
 - FIELD 4 is *frame number*. This must be an integer that increases with every row (frame). It's okay to miss/skip frames but never go backwards. In the future this could be a more complicated timestamp
 - Field 5 is a colon (:).  This marks the end of the header and beginning of actual `XYZ` data. A comma isn't needed between FIELD 4 and the colon.
 - Field 6-N is the data, laid out as defined in ['kpDescriptor.json'](#kpDescriptorjson).

An example of the first few rows in a CSV file represnting a single character might look like:
```file
mpii, 12th_man, , 0: 258.5, 2.61265, -72.5494, ...more keypoint XYZ data...
mpii, 12th_man, my custom metadata here, 1: 257.653, 1.93541, -74.9413, ...more keypoint XYZ data...
mpii, 12th_man, , 2: 257.836, 1.62057, -75.2355, ...more keypoint XYZ data...
```


## JSON
Not yet defined/implemented
 
## kpDescriptor.json
You can modify this JSON file to suit your needs, either adding new types or adjusting the order of joints. It looks something like this:
```json
{
   "mpii": {
      "description": "http://human-pose.mpi-inf.mpg.de/#download",
      "type": "humanoid",
      "layout": [
         "rightAnkle",
         "rightKnee",
         "rightHip",
         "leftHip",
         "leftKnee",
         "leftAnkle",
         "pelvis",
         "thorax",
         "upperNeck",
         "topHead",
         "rightWrist",
         "rightElbow",
         "rightShoulder",
         "leftShoulder",
         "leftElbow",
         "leftWrist"
      ]
   },
  "coco": {
      "description": "http://cocodataset.org/#download. Keypoint layout defined in person_keypoints_val2017.json from 'Train/Val annotations' dataset",
      "type": "humanoid",
      "layout": [
         "nose",
         "leftEye",
         "rightEye",
         "leftEar",
         "rightEar",
         "leftShoulder",
         "rightShoulder",
         "leftElbow",
         "rightElbow",
         "leftWrist",
         "rightWrist",
         "leftHip",
         "rightHip",
         "leftKnee",
         "rightKnee",
         "leftAnkle",
         "rightAnkle"
      ]
   }
}
```
