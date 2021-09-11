using UnityEngine.Playables;

namespace Riggingtools.Skeletons
{
    [System.Serializable]
    public class BRRigMoveBehaviour : PlayableBehaviour
    {
        public BRRigMoveManager brRigMoveManager = null;

        public override void ProcessFrame(Playable playable, FrameData info, object playerData)
        {
            if(brRigMoveManager != null)
            {
                brRigMoveManager.ProcessFrame();
            }
        }
    }
}