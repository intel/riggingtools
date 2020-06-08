#ifndef _MY_UTILITY_ // Don't use _UTILITY_ because it conflicts with a Windows define
#define _MY_UTILITY_

#include <unordered_map>
#include <string>
#include <vector>
#include <chrono>
#include <eigen3/Eigen/Geometry>
#include "rig2c.h"

extern std::string g_url;

#define CLIP(a) std::max(-1.0, std::min(a, 1.0))

#if defined(__GNUC__)
   #define strerror_s(buf,sz,err) strerror_r(err,buf,sz)
#endif

class Utility
{
public:

   static Utility * GetInstance();
       
   // Shared object helpers
   void LoadLib();
   void CloseLib();
   std::unordered_map< std::string, void * > & GetFunctions() { return _functions; }
   
   // File system helpers
   static std::string ExpandTilde( std::string directory );
   static bool DirectoryExists( std::string directory );
   static bool IsDirectory( std::string directory );
   static std::vector< std::string >GetFiles( std::string & directory,
      std::string mask );
   static std::string StripFilenameExtension( const std::string filename );
   static std::string GetFilenameExtension( const std::string filename );
   static bool GetApplicationDirectory( std::string & directory );
   
   // Eigen <--> std::array helpers
   static Eigen::Vector3d RawToVector( const std::array< double, 3 > & rawArray );
   static std::array< double, 3 > VectorToRaw( const Eigen::Vector3d & vec );
   static Eigen::Quaterniond RawToQuaternion( const std::array< double, 4 > & rawArray );
   static std::array< double, 4 > QuaternionToRaw( const Eigen::Quaterniond & quat );
   
   // Geometry helpers
   static std::vector< Eigen::Vector3d > QuadraticBezierCurve( Eigen::Vector3d p0,
      Eigen::Vector3d p1,
      Eigen::Vector3d p2,
      int numInterpolatedPoints,
      double ratio = 0.5 );
   static Eigen::Quaterniond Distance( Eigen::Quaterniond & a,
      Eigen::Quaterniond & b );
   
private:
   // Internal helpers
#ifndef _WIN32
   static int FileFilter ( const struct dirent * in_directoryEntry );
#endif
   static void BuildRegularExpression( char * inout_regEx, size_t bufLength );
   
   void * _libraryHandle = nullptr;
   std::unordered_map< std::string, void * > _functions;
};

// Generic RAII object
struct RAII
{
   RAII( std::function< void() > f ) { _f = f; }
   ~RAII() { _f(); }
private:
   std::function< void() > _f;
};

#endif
