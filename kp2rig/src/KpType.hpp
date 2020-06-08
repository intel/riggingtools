#ifndef KP_Type_h
#define KP_Type_h

#include <string>
#include <unordered_map>

// This enumeration tries to include anything on a humanoid that can be used as a keypoint.
// Non-humanoid keypoints are mapped to these; for example, a ball has only a "pelvis" keypoint.
enum KEYPOINT_TYPE
{
   RIGHT_ANKLE,
   RIGHT_KNEE,
   RIGHT_HIP,
   LEFT_HIP,
   LEFT_KNEE,
   LEFT_ANKLE,
   PELVIS,
   BASE_NECK,
   BASE_HEAD,
   TOP_HEAD,
   RIGHT_WRIST,
   RIGHT_ELBOW,
   RIGHT_SHOULDER,
   LEFT_SHOULDER,
   LEFT_ELBOW,
   LEFT_WRIST,
   LEFT_TOE,
   RIGHT_TOE,
   LEFT_HEEL,
   RIGHT_HEEL,
   NOSE,
   LEFT_EYE,
   RIGHT_EYE,
   LEFT_EAR,
   RIGHT_EAR,
   BACKGROUND,
   KP_UNKNOWN
};

inline KEYPOINT_TYPE StrToKpType( std::string str )
{
   static std::unordered_map< std::string, KEYPOINT_TYPE > map = {
      { "rightAnkle",    RIGHT_ANKLE    },
      { "rightKnee",     RIGHT_KNEE     },
      { "rightHip",      RIGHT_HIP      },
      { "leftHip",       LEFT_HIP       },
      { "leftKnee",      LEFT_KNEE      },
      { "leftAnkle",     LEFT_ANKLE     },
      { "pelvis",        PELVIS         },
      { "baseNeck",      BASE_NECK      },
      { "baseHead",      BASE_HEAD      },
      { "topHead",       TOP_HEAD       },
      { "rightWrist",    RIGHT_WRIST    },
      { "rightElbow",    RIGHT_ELBOW    },
      { "rightShoulder", RIGHT_SHOULDER },
      { "leftShoulder",  LEFT_SHOULDER  },
      { "leftElbow",     LEFT_ELBOW     },
      { "leftWrist",     LEFT_WRIST     },
      { "leftToe",       LEFT_TOE       },
      { "rightToe",      RIGHT_TOE      },
      { "leftHeel",      LEFT_HEEL      },
      { "rightHeel",     RIGHT_HEEL     },
      { "nose",          NOSE           },
      { "leftEye",       LEFT_EYE       },
      { "rightEye",      RIGHT_EYE      },
      { "leftEar",       LEFT_EAR       },
      { "rightEar",      RIGHT_EAR      },
      { "background",    BACKGROUND     }
   };
   
   auto it = map.find( str );
   if ( it == map.end() )
      return KP_UNKNOWN;
   else
      return (*it).second;
}

inline std::string KpTypeToStr( KEYPOINT_TYPE type )
{
   static std::unordered_map< KEYPOINT_TYPE, std::string > map = {
      { RIGHT_ANKLE,    "rightAnkle"    },
      { RIGHT_KNEE,     "rightKnee"     },
      { RIGHT_HIP,      "rightHip"      },
      { LEFT_HIP,       "leftHip"       },
      { LEFT_KNEE,      "leftKnee"      },
      { LEFT_ANKLE,     "leftAnkle"     },
      { PELVIS,         "pelvis"        },
      { BASE_NECK,      "baseNeck"      },
      { BASE_HEAD,      "baseHead"      },
      { TOP_HEAD,       "topHead"       },
      { RIGHT_WRIST,    "rightWrist"    },
      { RIGHT_ELBOW,    "rightElbow"    },
      { RIGHT_SHOULDER, "rightShoulder" },
      { LEFT_SHOULDER,  "leftShoulder"  },
      { LEFT_ELBOW,     "leftElbow"     },
      { LEFT_WRIST,     "leftWrist"     },
      { LEFT_TOE,       "leftToe"       },
      { RIGHT_TOE,      "rightToe"      },
      { LEFT_HEEL,      "leftHeel"      },
      { RIGHT_HEEL,     "rightHeel"     },
      { NOSE,           "nose"          },
      { LEFT_EYE,       "leftEye"       },
      { RIGHT_EYE,      "rightEye"      },
      { LEFT_EAR,       "leftEar"       },
      { RIGHT_EAR,      "rightEar"      },
      { BACKGROUND,     "background"    }
   };
   
   auto it = map.find( type );
   if ( it == map.end() )
      return "unknown";
   else
      return (*it).second;
}

#endif
