using UnityEngine;

namespace Riggingtools.Skeletons
{
    // ELF From VOLA source
    public class FrameTimeSource : MonoBehaviour
    {
        // Start is called before the first frame update
        [Tooltip("Current frame playing")]
        public int frame = 0;
        [Tooltip("Frame to start from")]
        public int start_frame = 0;
        [Tooltip("End frame (inclusive)")]
        public int end_frame = 0;
        [Tooltip("Framerate")]
        public float frames_per_second = 30.0f;
        public bool looping = true;
        [Tooltip("How long to delay when looping, in seconds")]
        public float loop_delay = 1.0f;// loop delay in seconds
        [Tooltip("Delay countdown, in seconds")]
        public float loop_delay_countdown = 1.0f;
        public bool paused = false;

        public bool random = false;
        [Tooltip("Show default player control buttons on the top left of the UI.")]
        public bool ShowGUIButtons = true;

        System.Random rng = new System.Random();

        float t = 0;// keeps fractional time

        int updated_frame = 0;

        void MyUpdate()
        {
            if (!Application.isPlaying) return;
            if (Time.frameCount == updated_frame) return;
            updated_frame = Time.frameCount;
            if (paused) return;

            if (random)
            {
                frame = rng.Next(start_frame, end_frame + 1);
            }

            float dt = Time.deltaTime;
            if (loop_delay_countdown > 0.0f)
            {
                loop_delay_countdown -= dt;
                return;
            }
            else
            {
                // adjust overshoot
                dt += loop_delay_countdown;
                loop_delay_countdown = 0.0f;
            }

            float fps = frames_per_second;

            bool backwards = false;
            if (fps < 0)
            {
                fps = -fps;
                backwards = true;
            }
            if (fps < 1E-6) return;
            t += dt;
            int d_frame = (int)Mathf.Floor(t * fps);
            t -= d_frame / fps;

            frame += backwards ? -d_frame : d_frame;

            if (frame < start_frame)
            {
                frame = end_frame;
                loop_delay_countdown = loop_delay;
            }
            else
            if (frame > end_frame)
            {
                frame = start_frame;
                loop_delay_countdown = loop_delay;
            }
        }

        void Start()
        {
            frame = start_frame;
        }

        // Update is called once per frame
        void Update()
        {
            if (Application.isPlaying)
            {
                MyUpdate();
                if (Input.GetKeyDown("space"))
                {
                    paused = !paused;
                }
            }
        }

        public void MovePlayersToFrame(int Sentframe)
        {
            frame = Sentframe;
            //MovePlayers();
        }

        public int GetFrame()
        {
            MyUpdate();
            return frame;
        }

        public void OnGUI()
        {
            if (!ShowGUIButtons) return;
            GUILayout.BeginVertical("Box");
            if (GUILayout.Button(paused ? "Resume" : "Pause"))
            {
                paused = !paused;
            }
            if (GUILayout.Button("Play 1 fps"))
            {
                paused = false;
                frames_per_second = 1;
            }
            if (GUILayout.Button("Play 5 fps"))
            {
                paused = false;
                frames_per_second = 5;
            }
            if (GUILayout.Button("Play 30 fps"))
            {
                paused = false;
                frames_per_second = 30;
            }
            if (GUILayout.Button("+1 frame"))
            {
                frame += 1;
                if (frame >= end_frame) frame = start_frame;
            }
            if (GUILayout.Button("-1 frame"))
            {
                frame -= 1;
                if (frame < start_frame) frame = end_frame;
            }
            GUILayout.EndVertical();
        }
    }
}
