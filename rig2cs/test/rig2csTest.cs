using System;
using System.Collections.Generic;
using System.Linq;

#if UNITY_5_3_OR_NEWER
   using UnityEngine;
#endif

public class ExampleUsage
{
   const int MAX_FRAMES = 10;

   class Character
   {
      public string name = "";
      public int startFrame = 9999999;
      public int endFrame = 0;
      public int numFrames = 0;
   }

   void BasicUsage(String url)
   {
      Dictionary<string, Character> characters = new Dictionary<string, Character>();
      int globalStartFrame = 9999999, globalEndFrame = 0;

      // Create a persistent object, representing all data for 
      // all rigs for all time - could be a singleton
      rig2cs rigStream = new rig2cs();

      // Call this once on startup
      try
      {
         rigStream.Initialize();
      }
      catch (Exception)
      {
         throw;
      }

      // Walk the rest post hierarchy
      rig2cs.Joint joint = rig2cs.getRestPoseHumanoid("");
      while (joint != null)
      {
         string name = joint.name;
         string parent = joint.parentName;
         double[] offset = joint.offset;
         double[] quat = joint.quaternion;
         int index = joint.index;
         joint = rig2cs.getRestPoseHumanoid(joint.name);
      }

      // Register callbacks once. Using lambdas for example purposes.
      rig2cs.Error += (rigId, code, description) =>
      {
         // TODO: Handle error
         System.Console.WriteLine("rig2c: ERROR (" + code + "): " + description);
      };
      rig2cs.Bounds += (rigId,
          startTimestamp,
          endTimestamp) =>
      {
         string type = rigStream.GetRigInfo(rigId, "type");
         string name = rigStream.GetRigInfo( rigId, "name" );

         // Set the global start/end frame, if necessary
         if (startTimestamp < globalStartFrame)
            globalStartFrame = startTimestamp;
         if (endTimestamp > globalEndFrame)
            globalEndFrame = endTimestamp;
      };
      rig2cs.Frame += (rigId,
          timestamp,
          position,
          rotations,
          boneLengths,
          boneOffsets ) =>
      {
         Character character;
         if (characters.ContainsKey(rigId))
         {
            character = characters[rigId];
         }
         else
         {
            character = new Character();
            character.name = rigId;
            characters[rigId] = character;
         }

         character.numFrames += 1;

         if (timestamp < character.startFrame)
            character.startFrame = timestamp;
         if (timestamp > character.endFrame)
            character.endFrame = timestamp;
      };

      // Start getting data and making above callbacks. This is asynchronous (non-blocking),
      // so all callbacks happen on an internal worker thread created by the library.
      rigStream.Start(url);

      // Normally you wouldn't call Stop() until exiting the app.
      // Remember, Start() will not block, hence the usage of Sleep() in this example.
      System.Threading.Thread.Sleep(500);

      string version = rigStream.GetInfo("version");
      string fps = rigStream.GetInfo("fps");

      rigStream.Stop();

      // Print a summary
      if (characters.Count > 0)
      {
         int averageNumFramesPerCharacter = 0;
         foreach( KeyValuePair<string, Character> pair in characters )
         {
            averageNumFramesPerCharacter += pair.Value.numFrames;
         }
         averageNumFramesPerCharacter = (int)((double)averageNumFramesPerCharacter / (double)characters.Count);
         Console.WriteLine("------------------------------------------------------");
         Console.WriteLine("Approx " + averageNumFramesPerCharacter + " frames per character, " +
            characters.Count + " characters, " +
            "bounds " + globalStartFrame + "->" + globalEndFrame);
      }
      // Call this once before exiting, it cleans up all the native pointers and such
      rigStream.Dispose();
   }

   static public void Main(String[] args)
   {
      // Assume the first argument is the json file
      if (args.Count() < 1)
      {
         Console.WriteLine("No JSON file specified");
         return;
      }
         
#if UNITY_5_3_OR_NEWER
      string jsonFilename = Application.streamingAssetsPath + "/test/seg_551.json";
#else
      string jsonFilename = args[0];
#endif

      ExampleUsage c = new ExampleUsage();

      c.BasicUsage(jsonFilename);
   }
}

