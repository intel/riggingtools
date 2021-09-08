using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SkeletonV2 : ISkeleton
{
    private bool verbose = false;

    public string character;
    public int timestamp { get; set; }
    public Vector3 position;
    public Dictionary<int, Quaternion> rotationsInternal;

    public Dictionary<int, double> boneLengths;
    public Dictionary<int, Vector3> boneOffsets;
    public double unitMeters;

    public SkeletonV2(FrameReport report, bool inVerbose = false)
    {
        verbose = inVerbose;

        character = report.character;
        timestamp = report.timestamp;
        //unitMeters = report.unitMeters;
        position = new Vector3((float)report.position.x, (float)report.position.y, (float)report.position.z);
        rotations = new Dictionary<int, Quaternion>();

        if (verbose)
        {
            Debug.LogFormat("<color=pink>CHARACTER: {0}  timestamp: {1}  unitMeters: {2}  position {3}</color>", character, timestamp, unitMeters, position.ToString());
        }

        for (int i = 0; i < report.rotations.Length; i++)
        {
            rotations.Add(i, new Quaternion((float) report.rotations[i].x, (float) report.rotations[i].y, (float) report.rotations[i].z, (float) report.rotations[i].w));
            if (verbose)
            {
                Debug.LogFormat("<color=orange>    Rotation: {0} = {1} (Euler {2} )</color>", i, rotations[i].ToString(), rotations[i].eulerAngles.ToString());
            }
        }

        boneLengths = new Dictionary<int, double>();
        for (int i = 0; i < report.boneLengths.Length; i++)
        {
            boneLengths.Add(i, report.boneLengths[i]);
            if (verbose)
            {
                Debug.LogFormat("<color=orange>    BoneLength: {0} = {1}</color>", i, boneLengths[i].ToString());
            }
        }
        boneOffsets = new Dictionary<int, Vector3>();
        for (int i = 0; i < report.boneOffsets.Length; i++)
        {
            boneOffsets.Add(i, new Vector3((float)report.boneOffsets[i].x, (float)report.boneOffsets[i].y, (float)report.boneOffsets[i].z));
            if (verbose)
            {
                Debug.LogFormat("<color=orange>    BoneOffset: {0} = {1}</color>", i, boneOffsets[i].ToString());
            }
        }
    }

    public enum JointIndex : int
    {
        Pelvis  = 0,
        RightHip = 1,
        RightKnee = 2,
        RightAnkle = 3,
        RightToebase = 4,
        LeftHip = 5,
        LeftKnee = 6,
        LeftAnkle = 7,
        LeftToebase = 8,
        Spine2 = 9,
        Spine3 = 10,
        Spine4 = 11,
        RightShoulder = 12,
        RightElbow = 13,
        RightWrite = 14,
        LeftShoulder = 15,
        LeftElbow = 16,
        LeftWrist = 17,
        Torso = 18,
        Neck = 19,
        TOTAL_JOINTS = 20
    }

    public Vector3 rootPosition
    {
        get
        {
            return position;
        }
    }
    public Dictionary<int, Vector3> positions
    {
        get
        {
            return boneOffsets;
        }
    }
    public Dictionary<int, Vector3> rotationsEuler { get; }
    public Dictionary<int, Quaternion> rotations { get; private set; }
}
