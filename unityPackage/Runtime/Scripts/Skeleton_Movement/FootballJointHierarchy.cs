using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FootballJointHierarchy : BaseJointHierarchy
{
    [SerializeField, Tooltip("Whether or not to auto-spin the ball when it is moving at a rate indicating that it is in the air.")]
    private bool autoSpin = true;
    [SerializeField, Tooltip("The frame-to-frame distance threshold that should be taken as indicating that the ball is airborne.")]
    private float spinThreshold = 3f;
    [SerializeField, Tooltip("The spin rate of the ball in Euler degrees per second.  Cursory research indicates that an American football spins about 10 times a second when thrown by a professional QB.")]
    private Vector3 spinRate = new Vector3(0f, 0f, 3600f);

    private float currentSpin = 0f;

    public override void AnimateFrame(ISkeleton frame, float blend)
    {
        if (frame != null)
        {
            if (!gameObject.activeInHierarchy)
            {
                gameObject.SetActive(true);
                prevFrame = frame;
            }

            if (prevFrame == null)
            {
                prevFrame = frame;
            }
            if (currentFrame == null)
            {
                currentFrame = frame;
            }
            else
            {
                if (currentFrame.timestamp != frame.timestamp)
                {
                    prevFrame = currentFrame;
                    currentFrame = frame;
                }
            }

            if (root != null)
            {
                root.transform.position = Vector3.Lerp(
                    prevFrame.rootPosition,
                    frame.rootPosition,
                  blend);
            }
        }

        float distance = Vector3.Distance(currentFrame.rootPosition, prevFrame.rootPosition);

        if (autoSpin && (distance > spinThreshold))
        {
            root.transform.rotation = Quaternion.FromToRotation(prevFrame.rootPosition, currentFrame.rootPosition);
            currentSpin += Time.deltaTime;
            root.transform.Rotate(spinRate * currentSpin);
        }
        else
        {
            currentSpin = 0;
            if (frame != null && frame.rotations != null && frame.rotations.Count > 0)
            {
                root.transform.rotation = frame.rotations[0];
            }
        }
    }

    public override void BackDoorEnterCorrections(List<Vector3> corrections)
    {
    }
}
