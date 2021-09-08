# Unity Skeleton Visualizer

## Description

A simple unity package that will allow you to visualize Skeleton json data within Unity.

## How-To

First, copy the rig2cs folder from riggingtools repo into your unity project.

Example: Scripts/rig2cs

Then:

Add the git URL to your unity project by going to Window > Package Manager > Click + > Add package from git URL > Paste URL

OR

Pull repo and point to the package.json by going to Window > Package Manager > Click + > Add package from file > navigate to package.json

## Additional Information

Create a StreamingAssets folder in the root of the Assets folder. Place all or any of your skeleton.json files within that folder and then update the file name in the PluginLoader.cs script within the inspector.

Check Packages/Skeleton Loader/Runtime/Scenes/SampleScene_SkeletonPlayer for demo setup.

EXPERIMENTAL_SkeletonPlayerEditor is experimental. Still needs work, but allows you to preview and manipulate the skeletons using Unity's Timeline window within the Editor. 
