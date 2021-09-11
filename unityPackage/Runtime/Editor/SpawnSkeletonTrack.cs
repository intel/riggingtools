/*
 * Experimental script. This scripts creates a Generate Cinemachine Skeleton Track button in the Experimental scene.
 */

using UnityEngine;
using UnityEditor;

namespace Riggingtools.Skeletons
{
    [CustomEditor(typeof(BRRigMoveManager))]
    public class SpawnSkeletonTrack : Editor
    {
		public override void OnInspectorGUI()
		{
			DrawDefaultInspector();

			BRRigMoveManager moveManager = (BRRigMoveManager)target;
			if(GUILayout.Button("Generate Skeleton Track"))
			{
				moveManager.AddPluginLoaderMoveTrack();
			}
		}
	}
}
