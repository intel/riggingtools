using Cinemachine;
using UnityEngine;

namespace Riggingtools.Skeletons
{
    public class AssignVCamToPlayerEditor : MonoBehaviour
    {
        public int cameraType = 0;

        public GameObject playerLoader;
        public GameObject vCamTypePrefab;
        public GameObject vCamGroupHolder;

        private int playerCount = 0;

        public void WaitForPlayersToSpawn()
        {
            if(this.gameObject.transform.childCount > 0)
			{
                return;
			}
            if (playerLoader != null)
            {
                playerCount = playerLoader.transform.childCount;
                Debug.Log("Child Count: " + playerCount);

                for (int i = 0; i < playerCount; i++)
                {
                    if (playerLoader.transform.GetChild(i) != null)
                    {
                        if (playerLoader.transform.GetChild(i).tag == "Player" || playerLoader.transform.GetChild(i).tag == "Ball")
                        {
                            GameObject vCams = Instantiate(vCamTypePrefab, vCamGroupHolder.transform.position, vCamGroupHolder.transform.rotation);
                            vCams.transform.parent = vCamGroupHolder.transform;


                            switch (cameraType)
                            {
                                case 0:
                                    vCams.name = playerLoader.transform.GetChild(i).name + "_FPS_VCam";
                                    if (playerLoader.transform.GetChild(i).tag == "Player")
                                    {
                                        vCams.GetComponent<CinemachineVirtualCamera>().Follow = playerLoader.transform.GetChild(i).GetChild(0).transform.GetChild(2).transform.GetChild(0).transform.GetChild(0).transform.GetChild(1).GetChild(0).transform;
                                        vCams.GetComponent<CinemachineVirtualCamera>().LookAt = playerLoader.transform.GetChild(i).GetChild(0).transform.GetChild(2).transform.GetChild(0).transform.GetChild(0).transform.GetChild(1).GetChild(0).transform;
                                    }
                                    else
                                    {
                                        vCams.GetComponent<CinemachineVirtualCamera>().Follow = playerLoader.transform.GetChild(i).transform;
                                        vCams.GetComponent<CinemachineVirtualCamera>().LookAt = playerLoader.transform.GetChild(i).transform;
                                    }
                                    break;
                                case 1:
                                    vCams.name = playerLoader.transform.GetChild(i).name + "_TPS_VCam";
                                    if (playerLoader.transform.GetChild(i).tag == "Player")
                                    {
                                        vCams.GetComponent<CinemachineVirtualCamera>().Follow = playerLoader.transform.GetChild(i).GetChild(0).transform;
                                        vCams.GetComponent<CinemachineVirtualCamera>().LookAt = playerLoader.transform.GetChild(i).GetChild(0).transform;
                                    }
                                    else
                                    {
                                        vCams.GetComponent<CinemachineVirtualCamera>().Follow = playerLoader.transform.GetChild(i).transform;
                                        vCams.GetComponent<CinemachineVirtualCamera>().LookAt = playerLoader.transform.GetChild(i).transform;
                                    }
                                    break;
								case 2:
                                    vCams.transform.Rotate(90.0f, 180.0f, 0.0f);
                                    vCams.name = playerLoader.transform.GetChild(i).name + "_Drone_VCam";
                                    if (playerLoader.transform.GetChild(i).tag == "Player")
                                    {
                                        vCams.GetComponent<CinemachineVirtualCamera>().Follow = playerLoader.transform.GetChild(i).GetChild(0).transform;
                                        vCams.GetComponent<CinemachineVirtualCamera>().LookAt = playerLoader.transform.GetChild(i).GetChild(0).transform;
                                    }
                                    else
                                    {
                                        vCams.GetComponent<CinemachineVirtualCamera>().Follow = playerLoader.transform.GetChild(i).transform;
                                        vCams.GetComponent<CinemachineVirtualCamera>().LookAt = playerLoader.transform.GetChild(i).transform;
                                    }
                                    break;
                                case 3:
                                    vCams.name = playerLoader.transform.GetChild(i).name + "_FrontTPS_VCam";
                                    if (playerLoader.transform.GetChild(i).tag == "Player")
                                    {
                                        vCams.GetComponent<CinemachineVirtualCamera>().Follow = playerLoader.transform.GetChild(i).GetChild(0).transform;
                                        vCams.GetComponent<CinemachineVirtualCamera>().LookAt = playerLoader.transform.GetChild(i).GetChild(0).transform;
                                    }
                                    else
                                    {
                                        vCams.GetComponent<CinemachineVirtualCamera>().Follow = playerLoader.transform.GetChild(i).transform;
                                        vCams.GetComponent<CinemachineVirtualCamera>().LookAt = playerLoader.transform.GetChild(i).transform;
                                    }
                                    break;
                                default:
                                    Debug.Log("Not a valid cameraType number assigned!");
                                    break;
                            }
                        }
                    }
                }
            }
        }
    }
}
