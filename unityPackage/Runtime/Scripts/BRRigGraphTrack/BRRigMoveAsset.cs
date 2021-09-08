using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace Riggingtools.Skeletons
{
    public class BRRigMoveAsset : PlayableAsset, ITimelineClipAsset
    {
        public ExposedReference<BRRigMoveManager> brRigMoveManager;

        public ClipCaps clipCaps { get { return ClipCaps.None; } }

        public override Playable CreatePlayable(PlayableGraph graph, GameObject owner)
        {
            var playable = ScriptPlayable<BRRigMoveBehaviour>.Create(graph);
            var behaviour = playable.GetBehaviour();
            behaviour.brRigMoveManager = brRigMoveManager.Resolve(graph.GetResolver());
            return playable;
        }
    }

}
