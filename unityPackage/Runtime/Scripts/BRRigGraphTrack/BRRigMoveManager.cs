using System;
using System.Collections;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Riggingtools.Skeletons
{
    public class BRRigMoveManager : MonoBehaviour
    {

        public int captureFrameRate = 30;
        public int currentFrame = 0;
        public PluginLoaderEditor RigMover;
		public FrameTimeSource frameTime;
        public PlayableDirector timeline;
        public bool showDebugLog = false;

		// Start is called before the first frame update
		/*
		private void Start()
		{
            StartCoroutine(AddPluginLoaderMoveTrack());
		}*/

		private void OnEnable()
		{
			
		}

		public void AddPluginLoaderMoveTrack()
		{
            /*
            while(frameTime.end_frame == 0)
			{
                yield return null;
			}*/

            TimelineAsset timelineAsset = timeline.playableAsset as TimelineAsset;
            timelineAsset.durationMode = TimelineAsset.DurationMode.FixedLength;
            timelineAsset.fixedDuration = frameTime.end_frame / frameTime.frames_per_second;
            foreach(var track in timelineAsset.GetOutputTracks())
			{
                timelineAsset.DeleteTrack(track);
			}
            BRRigMoveTrack rigMoveTrack = timelineAsset.CreateTrack<BRRigMoveTrack>();
            TimelineClip clip = rigMoveTrack.CreateClip<BRRigMoveAsset>();
            clip.duration = frameTime.end_frame / 30;
            clip.start = 0;
            BRRigMoveAsset rigMoveAsset = clip.asset as BRRigMoveAsset;
            timeline.SetReferenceValue(rigMoveAsset.brRigMoveManager.exposedName, RigMover.GetComponent<BRRigMoveManager>());
		}


        public void ProcessFrame()
        {
            if (timeline == null)
                return;

			currentFrame = Convert.ToInt32(timeline.time * captureFrameRate);

			if(currentFrame < frameTime.end_frame)
			{
				frameTime.MovePlayersToFrame(currentFrame);
			}

            if( currentFrame < RigMover.FrameCount)
            {
                // Debug.Log("Rendering current time: " + timeline.time + " frame # : " + currentFrame);

                //RigMover.MovePlayersToFrame(currentFrame);
            }
        }
    }
}