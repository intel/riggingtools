using Cinemachine;
using System.Collections;
using UnityEngine;

namespace Riggingtools.Skeletons
{
    public class AssignVCamToPlayer : MonoBehaviour
    {
        public enum VirtualCameraType
		{
            firstPersonCameras,
            thirdPersonCameras,
            droneFollowCameras,
            frontThirdPersonCameras
		}

        [Tooltip("What type of cameras would you like to create for the players and ball?")]
        public VirtualCameraType virtualCameraType;

        [Tooltip("This game object. If the field is null, it will automatically add it at Start()")]
        public GameObject vCamGroupHolder;
        [Tooltip("This is the skeleton gameobject that holds all of the players")]
        public GameObject skeletonLoader;
        [Tooltip("These are the different types of vCam Prefabs available. Only 4 are available currently.")]
        public GameObject[] vCamPrefabs;

        private GameObject vCamTypePrefab;

        private int playerCount = 0;

        // Start is called before the first frame update
        void Start()
        {
            if(!skeletonLoader)
			{
                skeletonLoader = GameObject.Find("SkeletonLoader");
                if(GameObject.Find("SkeletonLoader") == null)
				{
                    Debug.LogError("<color=red>ERROR: </color>SkeletonLoader prefab is missing or disabled. Did you accidentally rename or forget to add it to the scene?");
                }
                else
				{
                    Debug.Log("<color=green>The SkeletonLoader gameobject was found!</color>");
				}
			}
            if (!vCamGroupHolder)
            {
                vCamGroupHolder = this.gameObject;
            }
            else
			{
                Debug.Log("<color=green>VCamGroupHolder Gameobject already added</color>");
            }
            this.gameObject.name = virtualCameraType.ToString();

            switch (virtualCameraType.ToString())
			{
                case "firstPersonCameras":
                    vCamTypePrefab = vCamPrefabs[0];
                    break;
                case "thirdPersonCameras":
                    vCamTypePrefab = vCamPrefabs[1];
                    break;
                case "droneFollowCameras":
                    vCamTypePrefab = vCamPrefabs[2];
                    break;
                case "frontThirdPersonCameras":
                    vCamTypePrefab = vCamPrefabs[3];
                    break;
                default:
                    Debug.LogError("<color=red>ERROR: </color>NOT A VALID CAMERA TYPE!");
                    break;
            }
            StartCoroutine(WaitForPlayersToSpawn());
        }

        private IEnumerator WaitForPlayersToSpawn()
        {
            yield return null;

            while(!PluginLoader.isPlayersInstantiated)
			{
                yield return null;
			}

            if (skeletonLoader != null)
            {
                playerCount = skeletonLoader.transform.childCount;
                Debug.Log("Child Count: " + playerCount);

                for (int i = 0; i < playerCount; i++)
                {
                    if (skeletonLoader.transform.GetChild(i) != null)
                    {
                        if (skeletonLoader.transform.GetChild(i).tag == "Player" || skeletonLoader.transform.GetChild(i).tag == "Ball")
                        {
                            GameObject vCams = Instantiate(vCamTypePrefab, vCamGroupHolder.transform.position, vCamGroupHolder.transform.rotation);
                            vCams.transform.parent = vCamGroupHolder.transform;

                            yield return null;

                            switch (virtualCameraType.ToString())
                            {
                                case "firstPersonCameras":
                                    vCams.name = skeletonLoader.transform.GetChild(i).name + "_FPS_VCam";
                                    if (skeletonLoader.transform.GetChild(i).tag == "Player")
                                    {
                                        vCams.GetComponent<CinemachineVirtualCamera>().Follow = skeletonLoader.transform.GetChild(i).GetChild(0).transform.GetChild(2).transform.GetChild(0).transform.GetChild(0).transform.GetChild(1).GetChild(0).transform;
                                        vCams.GetComponent<CinemachineVirtualCamera>().LookAt = skeletonLoader.transform.GetChild(i).GetChild(0).transform.GetChild(2).transform.GetChild(0).transform.GetChild(0).transform.GetChild(1).GetChild(0).transform;
                                    }
                                    else
                                    {
                                        vCams.GetComponent<CinemachineVirtualCamera>().Follow = skeletonLoader.transform.GetChild(i).transform;
                                        vCams.GetComponent<CinemachineVirtualCamera>().LookAt = skeletonLoader.transform.GetChild(i).transform;
                                    }
                                    break;
                                case "thirdPersonCameras":
                                    vCams.name = skeletonLoader.transform.GetChild(i).name + "_TPS_VCam";
                                    if (skeletonLoader.transform.GetChild(i).tag == "Player")
                                    {
                                        vCams.GetComponent<CinemachineVirtualCamera>().Follow = skeletonLoader.transform.GetChild(i).GetChild(0).transform;
                                        vCams.GetComponent<CinemachineVirtualCamera>().LookAt = skeletonLoader.transform.GetChild(i).GetChild(0).transform;
                                    }
                                    else
                                    {
                                        vCams.GetComponent<CinemachineVirtualCamera>().Follow = skeletonLoader.transform.GetChild(i).transform;
                                        vCams.GetComponent<CinemachineVirtualCamera>().LookAt = skeletonLoader.transform.GetChild(i).transform;
                                    }
                                    break;
								case "droneFollowCameras":
                                    vCams.transform.Rotate(90.0f, 180.0f, 0.0f);
                                    vCams.name = skeletonLoader.transform.GetChild(i).name + "_Drone_VCam";
                                    if (skeletonLoader.transform.GetChild(i).tag == "Player")
                                    {
                                        vCams.GetComponent<CinemachineVirtualCamera>().Follow = skeletonLoader.transform.GetChild(i).GetChild(0).transform;
                                        vCams.GetComponent<CinemachineVirtualCamera>().LookAt = skeletonLoader.transform.GetChild(i).GetChild(0).transform;
                                    }
                                    else
                                    {
                                        vCams.GetComponent<CinemachineVirtualCamera>().Follow = skeletonLoader.transform.GetChild(i).transform;
                                        vCams.GetComponent<CinemachineVirtualCamera>().LookAt = skeletonLoader.transform.GetChild(i).transform;
                                    }
                                    break;
                                case "frontThirdPersonCameras":
                                    vCams.name = skeletonLoader.transform.GetChild(i).name + "_FrontTPS_VCam";
                                    if (skeletonLoader.transform.GetChild(i).tag == "Player")
                                    {
                                        vCams.GetComponent<CinemachineVirtualCamera>().Follow = skeletonLoader.transform.GetChild(i).GetChild(0).transform;
                                        vCams.GetComponent<CinemachineVirtualCamera>().LookAt = skeletonLoader.transform.GetChild(i).GetChild(0).transform;
                                    }
                                    else
                                    {
                                        vCams.GetComponent<CinemachineVirtualCamera>().Follow = skeletonLoader.transform.GetChild(i).transform;
                                        vCams.GetComponent<CinemachineVirtualCamera>().LookAt = skeletonLoader.transform.GetChild(i).transform;
                                    }
                                    break;
                                default:
                                    Debug.LogError("<color=red>ERROR: </color>FAILED TO CREATE THE REQUESTED CAMERAS!");
                                    break;
                            }
                        }
                    }
                }
            }
        }
    }
}
