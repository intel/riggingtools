using UnityEditor;
using UnityEngine;
using TMPro;

namespace Riggingtools.Skeletons
{
    [ExecuteAlways]
    public class PlayerIDDisplay : MonoBehaviour
    {
        public bool showPlayerID = false;
        [SerializeField]
        private GameObject playerRoot;
        [SerializeField]
        private GameObject playerSkeleton;
        [SerializeField]
        private GameObject playerCanvas;
        [SerializeField]
        private TMP_Text playerIDName;
        [SerializeField]
        private string id;

        // Start is called before the first frame update
        void Start()
        {
            playerIDName.text = playerSkeleton.name;
            id = playerIDName.text;
        }

        void LateUpdate()
        {
            playerCanvas.transform.position = playerRoot.transform.position;
            if (showPlayerID)
            {
                playerIDName.gameObject.SetActive(true);

                transform.LookAt(Camera.main.transform);
            }
            else
            {
                playerIDName.gameObject.SetActive(false);
            }
        }

#if UNITY_EDITOR
        private void OnDrawGizmos()
        {
            Vector3 namePosition = new Vector3(playerRoot.transform.position.x, 2.3f, playerRoot.transform.position.z);
            GUIStyle style = new GUIStyle();
            style.normal.textColor = Color.red;
            Handles.Label(namePosition, id, style);
        }
#endif
    }
}
