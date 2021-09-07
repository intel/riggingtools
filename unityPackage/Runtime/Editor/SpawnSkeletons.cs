/*
 * Experimental script. This scripts creates a Generate Skeletons button in the Experimental scene.
 */

using UnityEngine;
using UnityEditor;

namespace Riggingtools.Skeletons
{
    [CustomEditor(typeof(PluginLoaderEditor))]
    public class SpawnSkeletons : Editor
    {
		public override void OnInspectorGUI()
		{
			DrawDefaultInspector();

			PluginLoaderEditor loadSkeletons = (PluginLoaderEditor)target;
			if (GUILayout.Button("Generate Skeletons"))
			{
				loadSkeletons.GenerateSkeletons();
			}
		}
	}
}
