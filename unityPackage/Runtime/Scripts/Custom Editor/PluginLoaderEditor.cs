///<summary
///This version of the plugin loader is very experimental. When you add this prefab to your scene you need to make sure you have the URL setup correctly and then click the Generate Skeletons button. Afterwards, if you would like
///to control the skeletons with the Unity Timeline, you need to click the Generate Skeleton Track. Make sure you have the SkeletonTimeline in the scene and the Timeline field is populated with that gameobjects component.
///</summary>

using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Threading;
using UnityEngine;
using static rig2cs;
using Newtonsoft.Json;

namespace Riggingtools.Skeletons
{
    [ExecuteAlways]
    public class PluginLoaderEditor : MonoBehaviour
    {
        [SerializeField]
        private HeaderData headerData = new HeaderData();
        [SerializeField]
        private RigsData rigsData = new RigsData();

        [SerializeField]
        private int originalFrame = 0;
        [SerializeField]
        private int CurrentFrame = 0;

        public int FrameCount = -1;

        [SerializeField, Tooltip("The local file to initialize for the simulation.")]
        private string url = string.Empty;

        [SerializeField, Tooltip("The full path of the json file!")]
        private string appliedURL = string.Empty;

        [SerializeField, Tooltip("The default prefab key to instantiate if data specifies an invalid key.")]
        private string defaultPrefabKey = "player";

        [SerializeField, Tooltip("The prefabs for all players and other skeleton-animated objects.")]
        private List<PrefabSpec> prefabs;

        [SerializeField]
        private float followSharpness = 0.4f;

        [SerializeField]
        private FrameTimeSource timeSource = null;

        [SerializeField]
        private bool skinnedMeshVisible = false;

        [SerializeField]
        private bool playerIDVisible = false;

        [SerializeField]
        private bool verbose = false;

        [SerializeField]
        private bool globalCorrections = false;

        [SerializeField]
        private List<Vector3> corrections;

        [SerializeField]
        private int errorParsePerFrameMax = 15;
        [SerializeField]
        private int boundsParsePerFrameMax = 30;
        [SerializeField]
        private int frameParsePerFrameMax = 500;

        private rig2cs _rigwork;
        private static bool isPlayersInstantiated = false;

        private Queue<ErrorReport> errorQ;
        private Queue<BoundsReport> boundsQ;
        private Queue<FrameReport> frameQ;

        private Dictionary<string, BoundsReport> frameBounds;
        private Dictionary<string, int> framesReceived;
        private Dictionary<string, SkeletonV2[]> frameData;

        private Dictionary<string, HumanoidJointHierarchy> players;

        private Dictionary<string, GameObject> prefabRepo;

        private float lastUpdate;

        private Mutex errorQMutex;
        private Mutex framesQMutex;
        private Mutex boundsQMutex;

        private string refereeIndex = "5";

        private HumanoidJointHierarchy player = null;
        private FootballJointHierarchy ball = null;

        //[SerializeField]
        private Component[] skinMeshes;
        [SerializeField]
        private Component[] playerIds;
        private bool addComponents = true;

        private StreamReader reader;

        public void GenerateSkeletons()
		{
            if(this.gameObject.transform.childCount > 0)
			{
                return;
			}
            StartCoroutine(ParseResponses());
        }

        // Start is called before the first frame update
        private void Start()
        {
            _rigwork = new rig2cs();
            errorQ = new Queue<ErrorReport>();
            boundsQ = new Queue<BoundsReport>();
            frameQ = new Queue<FrameReport>();
            errorQMutex = new Mutex();
            framesQMutex = new Mutex();
            boundsQMutex = new Mutex();

            prefabRepo = new Dictionary<string, GameObject>();
            foreach (PrefabSpec spec in prefabs)
            {
                prefabRepo.Add(spec.key, spec.prefab);
            }

            if (_rigwork != null)
            {
                _rigwork.Initialize();
                rig2cs.Error += OnError;
                rig2cs.Frame += OnFrame;
                rig2cs.Bounds += OnBounds;
                frameBounds = new Dictionary<string, BoundsReport>();
                framesReceived = new Dictionary<string, int>();
                frameData = new Dictionary<string, SkeletonV2[]>();
                players = new Dictionary<string, HumanoidJointHierarchy>();

                appliedURL = Application.streamingAssetsPath + "/" + url;
                Debug.Log("Attempting to start from URL: " + appliedURL);
                lastUpdate = Time.time;

                if (!verbose)
                {
                    errorParsePerFrameMax = int.MaxValue;
                    boundsParsePerFrameMax = int.MaxValue;
                    frameParsePerFrameMax = int.MaxValue;
                }

                _rigwork.Start(appliedURL);
                if (timeSource == null)
                {
                    InvokeRepeating("MovePlayers", 1.0f, 0.033333333f);
                }
            }
            try
            {
                using (reader = new StreamReader(appliedURL))
                {
                    string json = reader.ReadToEnd();
                    headerData = JsonConvert.DeserializeObject<HeaderData>(json);
                    rigsData = JsonConvert.DeserializeObject<RigsData>(json);
                    if (headerData != null)
                    {
                        FrameCount = headerData.header.endFrame - headerData.header.startFrame + 1;
                        timeSource.end_frame = headerData.header.endFrame - headerData.header.startFrame + 1;
                    }
                }
            }
            catch
            {
                Debug.LogError("<color=red>Skeleton data parse failed!</color>");
            }

            skinMeshes = this.gameObject.GetComponentsInChildren(typeof(SkinnedMeshRenderer), true);
        }

        private void Update()
        {
            if (timeSource != null)
            {
                MovePlayers();
            }
            originalFrame = headerData.header.startFrame + timeSource.frame;
            if (originalFrame == (headerData.header.endFrame + 1))
            {
                originalFrame = headerData.header.endFrame;
            }

            if (isPlayersInstantiated && addComponents)
            {
                StartCoroutine(WaitForPlayersToAllLoad());
                addComponents = false;
            }
        }

        //Find the correct components for each player.
        IEnumerator WaitForPlayersToAllLoad()
        {
            yield return new WaitForEndOfFrame();

            skinMeshes = this.transform.GetComponentsInChildren(typeof(SkinnedMeshRenderer), true);
            playerIds = this.transform.GetComponentsInChildren(typeof(PlayerIDDisplay), true);
        }

        private void LateUpdate()
        {
            //Temporary override. Comment out if you'd like to use via UI button on screen.
            if (isPlayersInstantiated)
            {
                ToggleSkinnedMeshes(skinnedMeshVisible);
                ToggleIDText(playerIDVisible);
            }
        }

        public void MovePlayersToFrame(int frame)
        {
            CurrentFrame = frame;
            MovePlayers();
        }


        //Ability to show or hide the skin mesh of every player that is on the field.
        public void ToggleSkinnedMeshes(bool showSkin)
        {
            //TODO: Add ability to toggle individual teams. e.g. home, away or referee.
            if (showSkin)
            {
                foreach (SkinnedMeshRenderer skin in skinMeshes)
                {
                    skin.enabled = false;
                }
            }
            else
            {
                foreach (SkinnedMeshRenderer skin in skinMeshes)
                {
                    skin.enabled = true;
                }
            }
        }

        //Ability to show or hide the ID text at runtime. This is good in case you do not have specific jersey textures of the play.
        public void ToggleIDText(bool showID)
        {
            if (showID)
            {
                foreach (PlayerIDDisplay id in playerIds)
                {
                    id.showPlayerID = true;
                }
            }
            else
            {
                foreach (PlayerIDDisplay id in playerIds)
                {
                    id.showPlayerID = false;
                }
            }
        }

        private void MovePlayers()
        {
            float blend = 1f - Mathf.Pow(1f - followSharpness, Time.deltaTime * 30f);

            if (CurrentFrame == 0)
            {
                blend = 1;
            }

            // Animate the players that are on the field.
            if (players != null)
            {
                foreach (KeyValuePair<string, HumanoidJointHierarchy> playerData in players)
                {
                    string pName = playerData.Key;
                    HumanoidJointHierarchy player = playerData.Value;
                    if (originalFrame >= player.startFrame && originalFrame <= player.endFrame)
                    {
                        player.gameObject.SetActive(true);
                        if (framesReceived.ContainsKey(pName))
                        {
                            if (frameData.ContainsKey(pName))
                            {
                                SkeletonV2 frame = null;

                                int offsetFrame = player.startFrame - headerData.header.startFrame;

                                int absoluteFrame = CurrentFrame - offsetFrame;

                                if (frameData[pName].Length > absoluteFrame && absoluteFrame >= 0)
                                {
                                    frame = frameData[pName][absoluteFrame];
                                }

                                // ELF FUBAR Debug tweaking so that can be adjusted from editor
                                if (globalCorrections && corrections != null)
                                {
                                    (player as HumanoidJointHierarchy).BackDoorEnterCorrections(corrections);
                                }
                                player.AnimateFrame(frame, blend);
                            }
                        }
                    }
                    else
                    {
                        player.gameObject.SetActive(false);
                    }
                }
            }

            //Animate the ball if there is one present.
            if (ball != null)
            {
                SkeletonV2 frame = null;
                if (frameData["ball"].Length > CurrentFrame)
                {
                    frame = frameData["ball"][CurrentFrame];
                }
                if (frame != null)
                {
                    ball.AnimateFrame(frame, blend);
                }
            }

            if (timeSource != null)
            {
                //Change this based on if running in slow mo or rewind etc
                CurrentFrame = timeSource.frame + 1;

                //only run the below if time source's frame is out of bounds?
                if (CurrentFrame >= FrameCount)
                {
                    CurrentFrame = 0;
                }
            }
            else
            {
                CurrentFrame++;

                if (CurrentFrame >= FrameCount)
                {
                    CurrentFrame = 0;
                }
            }

        }

        // Update is called once per frame
        private IEnumerator ParseResponses()
        {
            int parseIterations = 0;

            while (_rigwork != null)
            {
                parseIterations = 0;

                errorQMutex.WaitOne();

                while (errorQ.Count > 0 && parseIterations < errorParsePerFrameMax)
                {
                    ErrorReport eReport = errorQ.Dequeue();
                    HandleError(eReport.character, eReport.errorCode, eReport.message);
                    parseIterations++;
                }

                errorQMutex.ReleaseMutex();

                parseIterations = 0;

                boundsQMutex.WaitOne();

                while (boundsQ.Count > 0 && parseIterations < boundsParsePerFrameMax)
                {
                    BoundsReport bReport = boundsQ.Dequeue();

                    Debug.Log("Total Frames: " + bReport.numFrames);


                    if (true)
                    {
                        Debug.Log("<color=green>OnBounds:  character name: " + bReport.character + " start: " + bReport.beginTime + " end: " + bReport.endTime + "</color>");
                    }
                    if (!frameBounds.ContainsKey(bReport.character))
                    {
                        for (int i = 0; i < rigsData.rigs.Length; i++)
                        {
                            if (bReport.character == rigsData.rigs[i].id)
                            {
                                if (rigsData.rigs[i].id == "ball" && rigsData.rigs[i].type == "solidObject")
                                {
                                    bReport.type = "solidObject";
                                }
                            }
                            else
                            {
                                bReport.type = defaultPrefabKey;
                            }
                        }

                        frameBounds.Add(bReport.character, bReport);
                        frameData.Add(bReport.character, new SkeletonV2[bReport.endTime - bReport.beginTime + 1]);

                        GameObject prefab;
                        if (prefabRepo.ContainsKey(bReport.type))
                        {
                            prefab = prefabRepo[bReport.type];
                        }
                        else
                        {
                            prefab = prefabRepo[defaultPrefabKey];
                        }

                        GameObject newPlayer = GameObject.Find(bReport.character);

                        if (newPlayer == null)
                        {
                            newPlayer = Instantiate(prefab);
                        }
                        else
                        {
                            GameObject reference = Instantiate(prefab);

                            for (int i = 0; i < newPlayer.GetComponent<HumanoidJointHierarchy>().joints.Count; i++)
                            {
                                newPlayer.GetComponent<HumanoidJointHierarchy>().joints[i].transform.localRotation = reference.GetComponent<BaseJointHierarchy>().joints[i].transform.localRotation;

                                newPlayer.GetComponent<HumanoidJointHierarchy>().joints[i].transform.localPosition = reference.GetComponent<BaseJointHierarchy>().joints[i].transform.localPosition;
                            }

                            GameObject.DestroyImmediate(reference);
                            //Reset them...? 
                        }

                        player = null;
                        ball = null;

                        if (bReport.type == defaultPrefabKey)
                        {
                            player = newPlayer.GetComponent<HumanoidJointHierarchy>();
                        }
                        else
                        {
                            ball = newPlayer.GetComponent<FootballJointHierarchy>();
                        }

                        if (player != null || ball != null)
                        {
                            newPlayer.transform.SetParent(this.transform);
                            newPlayer.transform.localPosition = Vector3.zero;
                            string teamId = bReport.character.Substring(bReport.character.Length - 1);
                            if (player != null && teamId == refereeIndex)
                            {
                                newPlayer.tag = "Referee";
                            }
                            else if (player != null)
                            {
                                newPlayer.tag = "Player";
                            }
                            else
                            {
                                newPlayer.tag = "Ball";
                            }
                        }

                        //This replaces the name of the gameobject inside of the hierarchy to that of the jersey number inside of json file.
                        newPlayer.name = bReport.character;

                        if (player != null)
                        {
                            player.Init(new Dictionary<string, object> { { "loader", this } }, null);
                            player.startFrame = bReport.beginTime;
                            player.endFrame = bReport.endTime;
                            players.Add(bReport.character, player);
                        }
                    }
                    lastUpdate = Time.time;
                    parseIterations++;
                }

                boundsQMutex.ReleaseMutex();

                parseIterations = 0;

                framesQMutex.WaitOne();

                while (frameQ.Count > 0 && parseIterations < frameParsePerFrameMax)
                {
                    FrameReport fReport = frameQ.Dequeue();
                    if (verbose)
                    {
                        Debug.LogFormat("<color=blue>OnFrame:  character name: {0} timestamp : {1} position {2} Rotation count {3} boneLengths count {4} boneOffsets count {5} unit meters {6} </color>",
                            fReport.character, fReport.timestamp, fReport.position, fReport.rotations.Length, fReport.boneLengths.Length, fReport.boneOffsets.Length);
                    }
                    if (frameBounds.ContainsKey(fReport.character))
                    {
                        if (!framesReceived.ContainsKey(fReport.character))
                        {
                            framesReceived.Add(fReport.character, 0);
                        }

                        framesReceived[fReport.character]++;
                        int index = fReport.timestamp - frameBounds[fReport.character].beginTime;
                        frameData[fReport.character][index] = new SkeletonV2(fReport, verbose);
                        lastUpdate = Time.time;

                        parseIterations++;
                    }
                    else
                    {
                        Debug.LogWarning("Received frame before bounds for character: " + fReport.character + ", requeueing and breaking frame response");
                        frameQ.Enqueue(fReport);
                        break;
                    }
                }

                framesQMutex.ReleaseMutex();


                if ((errorQ.Count == 0) && (boundsQ.Count == 0) && (frameQ.Count == 0) && ((Time.time - lastUpdate) > 0.5f))
                {
                    bool missingFrame = false;

                    foreach (string character in frameBounds.Keys)
                    {
                        if (framesReceived.ContainsKey(character))
                        {
                            if (framesReceived[character] < frameBounds[character].numFrames)
                            {
                                missingFrame = true;
                            }
                        }
                        else
                        {
                            missingFrame = true;
                        }
                    }

                    if (!missingFrame)
                    {
                        DisposeRigwork();
                    }
                }

                int num = 999999999;

                foreach (KeyValuePair<string, int> kvp in framesReceived)
                {
                    if (kvp.Value < num)
                    {
                        num = kvp.Value;
                    }
                }

                yield return null;

                isPlayersInstantiated = true;
            }
        }

        private void DisposeRigwork()
        {
            rig2cs.Bounds -= OnBounds;
            rig2cs.Error -= OnError;
            rig2cs.Frame -= OnFrame;
            _rigwork.Stop();
            _rigwork.Dispose();
            _rigwork = null;

            if (verbose)
            {
                Debug.Log("<color=green>Rigwork disposed.</color>");
            }
        }

        private void HandleError(string character, int errorCode, string description)
        {
            Debug.LogError("OnError:  Character: " + character + "  Code: " + errorCode.ToString() + " Description - " + description);
        }

        private void OnError(string rigId, int errorCode, string description)
        {
            errorQMutex.WaitOne();
            ErrorReport report = new ErrorReport(rigId, errorCode, description);
            errorQ.Enqueue(report);
            errorQMutex.ReleaseMutex();
        }

        private void OnBounds(string rigId, int startTimestamp, int endTimestamp)
        {
            boundsQMutex.WaitOne();
            string type = _rigwork.GetRigInfo(rigId, "type");
            BoundsReport report = new BoundsReport(rigId, type, startTimestamp, endTimestamp, endTimestamp - startTimestamp + 1);
            boundsQ.Enqueue(report);
            boundsQMutex.ReleaseMutex();
        }

        private void OnFrame(string rigId, int timestamp, Position position, Rotation[] rotations, double[] boneLengths, Position[] boneOffsets)
        {
            framesQMutex.WaitOne();
            FrameReport report = new FrameReport(rigId, timestamp, position, rotations, boneLengths, boneOffsets);
            frameQ.Enqueue(report);
            framesQMutex.ReleaseMutex();
        }

        private void OnApplicationQuit()
        {
            if (_rigwork != null)
            {
                DisposeRigwork();
            }
        }
    }
}
