using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public abstract class BaseJointHierarchy : MonoBehaviour, IJointHierarchy
{
	[Header("CENTER OF BODY")]
	public GameObject root;

	[Header("JOINTS (0 = pelvis)")]
	[SerializeField]
	public List<GameObject> joints;

	public List<Vector3> correctiveAngles;
	public List<GameObject> jointTrackers { get; set; }

	protected List<Vector3> initialAngles;

	protected ISkeleton prevFrame = null;
	protected ISkeleton currentFrame = null;

	public virtual void Init(Dictionary<string, object> config, GameObject prefab)
	{
		initialAngles = new List<Vector3>();
		for (int i = 0; i < joints.Count; i++)
		{
			if (joints[i] != null)
			{
				initialAngles.Add(joints[i].transform.localRotation.eulerAngles);
			}
			else
			{
				initialAngles.Add(Vector3.zero);
			}
		}
	}

	public abstract void AnimateFrame(ISkeleton frame, float blend);

	public abstract void BackDoorEnterCorrections(List<Vector3> corrections);
}
