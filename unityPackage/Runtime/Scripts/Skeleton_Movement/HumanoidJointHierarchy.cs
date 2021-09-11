using System.Collections.Generic;
using UnityEngine;

namespace Riggingtools.Skeletons
{
    public class HumanoidJointHierarchy : BaseJointHierarchy
    {
        [SerializeField]
        private GameObject jointTrackerAnchor;

        private PluginLoader _loader = null;
        private PluginLoaderEditor _loaderEditor = null;

        private new ISkeleton prevFrame = null;
        private new ISkeleton currentFrame = null;

        public bool playOnly = true;
        public int startFrame;
        public int endFrame;

        public override void Init(Dictionary<string, object> config, GameObject prefab)
        {
            //Removed the joint spheres from spawning.
            //InitJointTrackers(prefab);

            if (playOnly)
            {
                if (config.ContainsKey("loader"))
                {
                    _loader = (PluginLoader)config["loader"];
                }
            }
            else
            {
                if (config.ContainsKey("loader"))
                {
                    _loaderEditor = (PluginLoaderEditor)config["loader"];
                }
            }


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

                for (int i = 0; i < joints.Count; i++)
                {
                    // ELF FUBAR Gimbal lock from using eulers?
                    if (joints.Count >= i && initialAngles.Count >= i && joints[i] != null)
                    {
                        joints[i].transform.localRotation = Quaternion.Euler(initialAngles[i] + correctiveAngles[i]);
                        joints[i].transform.Rotate(Quaternion.Slerp(prevFrame.rotations[i], frame.rotations[i], blend).eulerAngles);
                    }
                }
            }
            else
            {
                if (gameObject.activeInHierarchy)
                {
                    //gameObject.SetActive(false);
                }
            }
        }

        public override void BackDoorEnterCorrections(List<Vector3> corrections)
        {
            correctiveAngles = corrections;
        }

        protected void InitJointTrackers(GameObject trackerPrefab)
        {
            if (jointTrackerAnchor == null)
            {
                jointTrackerAnchor = new GameObject("JointTrackerAnchor");
                jointTrackerAnchor.transform.SetParent(transform);
            }

            jointTrackers = new List<GameObject>();
            for (int j = 0; j < 20; j++)
            {
                if (trackerPrefab == null)
                {
                    jointTrackers.Add(GameObject.CreatePrimitive(PrimitiveType.Sphere));
                }
                else
                {
                    jointTrackers.Add(GameObject.Instantiate(trackerPrefab));
                }

                jointTrackers[j].name = ((SkeletonV2.JointIndex)j).ToString();
                jointTrackers[j].transform.SetParent(jointTrackerAnchor.transform);
            }
        }
    }
}
