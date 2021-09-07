using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public interface ISkeleton
{
    int timestamp { get; }
    Vector3 rootPosition { get; }
    Dictionary<int, Vector3> positions { get; }
    Dictionary<int, Vector3> rotationsEuler { get; }
    Dictionary<int, Quaternion> rotations { get; }
}
