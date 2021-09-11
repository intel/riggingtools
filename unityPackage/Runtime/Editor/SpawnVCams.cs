/*
 * Experimental script. This scripts creates a Generate Virtual Cameras button in the Experimental scene.
 */

using UnityEngine;
using UnityEditor;

namespace Riggingtools.Skeletons
{
	[CustomEditor(typeof(AssignVCamToPlayerEditor))]
	public class SpawnVCams : Editor
	{
		public override void OnInspectorGUI()
		{
			DrawDefaultInspector();

			AssignVCamToPlayerEditor assignVcams = (AssignVCamToPlayerEditor)target;
			if (GUILayout.Button("Generate Virtual Cameras"))
			{
				assignVcams.WaitForPlayersToSpawn();
			}
		}

	}
}
