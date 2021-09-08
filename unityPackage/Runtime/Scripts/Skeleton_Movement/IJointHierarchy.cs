using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public interface IJointHierarchy
{
    void Init(Dictionary<string, object> config, GameObject prefab);
    void AnimateFrame(ISkeleton frame, float blend);

    //void JointUpdateFromTrackers(Skeleton frame, float blend);
    //void RotateOnlyEuler(Skeleton frame, float blend);
    //void RotationOnlyNonEuler(Skeleton frame, float blend);
    //void RepositionJoints(Skeleton frame, float blend);

    //void InitJointTrackers(GameObject prefab);
    //void TrackJointsWithObjects(Skeleton frame, float blend);

    //PositionLoader loader { get; set; }
    List<GameObject> jointTrackers { get; }
    void BackDoorEnterCorrections(List<Vector3> corrections);
}
