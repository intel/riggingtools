#ifdef _WIN32
   #define WIN32_LEAN_AND_MEAN
   #define NOMINMAX
   #include <windows.h>
#else
   #if defined(__APPLE__)
      #include <mach-o/dyld.h>
      #include <TargetConditionals.h>
   #else
      #include <unistd.h>
   #endif
   
   #if !defined( __ANDROID__ )
      #include <wordexp.h>
   #endif
   
   #include <dirent.h>
   #include <regex.h>
   #include <sys/stat.h>
   #include <dlfcn.h>

   // Needed for filtering files
   regex_t * filter = nullptr;
#endif

#include <sstream>
#include <string>
#include <sys/stat.h>
#include <utility>
#include <unordered_map>
#include <fstream>
#include <stdio.h>
#include <thread>
#include <array>
#include <vector>
#include <stdexcept>
#include "Utility.hpp"
#include "rig2cDelegates.h"

// Globals
int g_initCounter = 0;
std::string g_url;

Utility * Utility::GetInstance()
{
   static Utility * instance = nullptr;
   if ( !instance )
   {
      instance = new Utility();
   }
   
   return instance;
}

// Shared object helpers
void Utility::LoadLib()
{
   std::vector< std::string > directories = { "./" };
   
   // Include the binary path when searching for the file
   std::string applicationDirectory;
   if ( Utility::GetApplicationDirectory( applicationDirectory ) )
   {
      if ( *(applicationDirectory.end() - 1) != '/' )
         applicationDirectory += "/";
      directories.push_back( applicationDirectory );
   }
   
   // Can add other paths here to search for the .so/dll if needed
      
#if defined(_WIN32)
   std::string libraryFilename = "rig2c.dll";
   for ( auto & directory : directories )
   {
      _libraryHandle = LoadLibrary( (directory + libraryFilename).c_str() );
      if ( _libraryHandle) break;
   }
   
   if (!_libraryHandle)
   {
      char errorMessage[512];
      FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
         NULL,
         GetLastError(),
         0,
         errorMessage,
         sizeof(errorMessage),
         NULL);
      throw std::runtime_error( errorMessage );
   }
      
#else
   const char * libraryFilename = "librig2c.so";
   for ( auto & directory : directories )
   {
      _libraryHandle = dlopen( (directory + libraryFilename).c_str(), RTLD_NOW );
      if ( _libraryHandle) break;
   }
   
   if ( !_libraryHandle )
      throw std::runtime_error( dlerror() );
#endif
   
   // Load known functions
   std::vector< std::string > functionNames = {
      "rig_initialize",
      "rig_uninitialize",
      "rig_setErrorCallback",
      "rig_setBoundsCallback",
      "rig_setFrameCallback",
      "rig_getLastError",
      "rig_getInfo",
      "rig_getRigInfo",
      "rig_read",
      "rig_startRead",
      "rig_stopRead"
   };
   for ( auto & functionName : functionNames )
   {
#if defined WIN32
      _functions.insert( std::make_pair( functionName, GetProcAddress( (HMODULE)_libraryHandle, functionName.c_str() ) ) );
#else
      _functions.insert( std::make_pair( functionName, dlsym( _libraryHandle, functionName.c_str() ) ) );
#endif
   }
}
void Utility::CloseLib()
{
   if ( _libraryHandle )
#if defined(_WIN32)
      FreeLibrary( (HMODULE)_libraryHandle );
#else
      dlclose( _libraryHandle );
#endif

   _libraryHandle = nullptr;
}

// File system helpers
std::string Utility::ExpandTilde( std::string directory )
{
   // If it starts with a tilde
   if ( directory[ 0 ] == '~' )
   {
#if (TARGET_OS_IPHONE) || (_WIN32) || (__ANDROID__)

      throw std::runtime_error( "OS does not support tilde character in paths" );
      
#else

      // Expand it out
      wordexp_t exp_result;
      wordexp( directory.c_str(), &exp_result, 0 );
      directory = std::string( exp_result.we_wordv[0] );
      
#endif
   }
   
   return directory;
}
bool Utility::DirectoryExists( std::string directory )
{
#ifdef _WIN32
   DWORD attributes = GetFileAttributes(directory.c_str());

   if (attributes == INVALID_FILE_ATTRIBUTES)
   {
      return false;
   }
   else
   {
      return (attributes & FILE_ATTRIBUTE_DIRECTORY) > 0;
   }
#else
   struct stat sb;
   
   // If empty
   if ( directory == "" )
   {
      // Yes, this is the working directory
      return true;
   }
   
   // Expand any tilde paths
   directory = ExpandTilde( directory );
   
   // Let's ask the file system
   if ( stat( directory.c_str(), &sb ) == 0 && S_ISDIR( sb.st_mode ) )
   {
      return true;
   }
   else
   {
      return false;
   }
#endif
}
bool Utility::IsDirectory( std::string directory )
{
   return DirectoryExists( directory );
}
std::vector< std::string > Utility::GetFiles( std::string & directory,
   std::string mask )
{
   std::vector< std::string > returnValue;
   
   // Expand tilde
   directory = ExpandTilde( directory );
   
   // If this isn't a directory at all
   if ( !IsDirectory( directory ) )
   {
      // Assume a file
      std::string file = directory;
      size_t lastSlash = directory.find_last_of( '/' );
      if ( lastSlash == std::string::npos )
         lastSlash = directory.find_last_of( '\\' );
      if ( lastSlash != std::string::npos )
      {
         // Get the filename only
         file = directory.substr( lastSlash + 1 );
         
         // Update the directory without the filename
         directory = directory.substr( 0, lastSlash + 1 );
      }
      
      // Add the filename only (without the path) and exit
      returnValue.push_back( file );
      return returnValue;
   }
   
#if defined(_WIN32)

   WIN32_FIND_DATA fileData;
   HANDLE h;

   std::string path = directory;

   // Ensure it ends with a forward slash
   if (path.back() != '\\')
      path += '\\';

   // If a mask was supplied
   if ( mask != "" )
   {
      path += mask;
   }
   else
   {
      path += "*";
   }

   h = FindFirstFile(path.c_str(), &fileData );

   if ( h != INVALID_HANDLE_VALUE )
   {
      do
      {
         // If this is a file
         if ( ( fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
         {
            returnValue.push_back( std::string( fileData.cFileName ) );
         }

      } while ( FindNextFile( h, &fileData ) );

      FindClose( h );
   }
   
#else

   int regError;
   int numFiles;
   struct dirent ** namelist;
   
   // If a mask was supplied
   if ( mask != "" )
   {
      char nativeMask[ 512 ];
      
      // Copy the mask and make a regular expression out of it
      strcpy( nativeMask, mask.c_str() );
      BuildRegularExpression( nativeMask, sizeof(nativeMask) );

      // Initialize the regular expression filter
      filter = new regex_t();
      if ( ( regError = regcomp( filter, nativeMask, REG_EXTENDED | REG_NOSUB ) ) != 0 )
      {
         char message[ 512 ];
         strcpy( message, "DiskUtility::GetFiles(): " );
         regerror( regError, filter, message + strlen( message ), 480 );
         throw std::runtime_error( message );
      }
   }

   // If the directory is empty
   if ( directory == "" )
   {
      // scandir is picky and needs a dot-slash
      directory = "./";
   }

   // Get the file list, filtered by our filter
   numFiles = scandir( directory.c_str(),
      &namelist,
      FileFilter,
      NULL );

   // If we used a regular expression filter
   if ( filter )
   {
      // Clean it up
      regfree( filter );
      delete filter;
      filter = NULL;
   }
   
   // For every file
   while( numFiles-- > 0 )
   {
      returnValue.push_back( std::string( namelist[ numFiles ]->d_name ) );
      free( namelist[ numFiles ] );
   }

   // If it didn't fail (-1 from scandir and then decremented once in while loop)
   if ( numFiles > -2 )
      free( namelist );

#endif
   
   return returnValue;
}
std::string Utility::StripFilenameExtension( const std::string filename )
{
   size_t dotIndex = filename.find_last_of( "." );
   size_t slashIndex = filename.find_last_of( "/" );
   
   if ( dotIndex &&
      (slashIndex == std::string::npos || dotIndex > slashIndex) )
   {
      return filename.substr( 0, dotIndex );
   }
   else
   {
      return filename;
   }
}
std::string Utility::GetFilenameExtension( const std::string filename )
{
   size_t dotIndex = filename.find_last_of( "." );
   size_t slashIndex = filename.find_last_of( "/" );
   
   if ( dotIndex &&
      (slashIndex == std::string::npos || dotIndex > slashIndex) )
   {
      return filename.substr( dotIndex + 1 );
   }
   else
   {
      return "";
   }
}
bool Utility::GetApplicationDirectory( std::string & directory )
{
   bool returnValue = true;
   
#if defined(WIN32)

   char buffer[ 512 ];
   if ( GetModuleFileName( NULL,
      buffer,
      (DWORD)sizeof( buffer) ) > 0 )
   {
      char * temp = strrchr( buffer, '\\' );
      *temp = 0;
      directory = std::string( buffer );
   }
   else
   {
      returnValue = false;
   }
     
#elif defined(__linux__)
 
   char buffer[ 512 ];
   ssize_t count = readlink( "/proc/self/exe",
      buffer,
      sizeof( buffer ) );

   // readlink does not NULL terminate it's strings
   buffer[ count ] = 0;

   char * temp = strrchr( buffer, '/' );
   if ( temp )
      *temp = 0;
     
   directory = std::string( buffer );
   
#elif defined(__APPLE__)
 
   char buffer[ 512 ];
   uint32_t bufsize = sizeof( buffer );
   if ( 0 != _NSGetExecutablePath( buffer, &bufsize ) )
   {
      returnValue = false;
   }
   else
   {
      directory = std::string( buffer );
   
      // This includes the application itself, so remove it
      directory = directory.substr( 0, directory.find_last_of( '/') );
   }
 
#endif

   return returnValue;
}

// Eigen <--> std::array helpers
Eigen::Vector3d Utility::RawToVector( const std::array< double, 3 > & rawArray )
{
   return Eigen::Vector3d( rawArray[ 0 ], rawArray[ 1 ], rawArray[ 2 ] );
}
std::array< double, 3 > Utility::VectorToRaw( const Eigen::Vector3d & vec )
{
   return std::array< double, 3 >( { vec[ 0 ], vec[ 1 ], vec[ 2 ] } );
}
Eigen::Quaterniond Utility::RawToQuaternion( const std::array< double, 4 > & rawArray )
{
   return Eigen::Quaterniond( rawArray[ 3 ], rawArray[ 0 ], rawArray[ 1 ], rawArray[ 2 ] );
}
std::array< double, 4 > Utility::QuaternionToRaw( const Eigen::Quaterniond & quat )
{
   return std::array< double, 4 >( { quat.x(), quat.y(), quat.z(), quat.w() } );
}

// Geometry helpers

// This is actually a rational curve with ratio [0.0, 1.0].
// Resulting segments are guaranteed to be close to the same length,
// hence the extra effort to populate a LUT
std::vector< Eigen::Vector3d > Utility::QuadraticBezierCurve( Eigen::Vector3d p0,
   Eigen::Vector3d p1,
   Eigen::Vector3d p2,
   int numInterpolatedPoints,
   double ratio )
{
   std::vector< Eigen::Vector3d > returnValue;
   
   double totalArcLength = 0.0;
   std::pair< double, Eigen::Vector3d > lut[ 100 ];
   for ( int i=0; i < 100; ++i )
   {
      double t = 0.01 * (i+1);
      double f[] = { pow(1.0-t, 2.0),
         ratio * 2.0*t*(1.0-t),
         pow(t, 2.0) };
      double basis = f[0] + f[1] + f[2];
      Eigen::Vector3d p = (f[0]*p0 + f[1]*p1 + f[2]*p2)/basis;
      double length;
      if ( i == 0 )
         length = (p - p0).norm();
      else
         length = (p - lut[i-1].second).norm();
      
      lut[i] = std::make_pair(length, p);
      totalArcLength += length;
   }
   
   double targetLength = totalArcLength / double(numInterpolatedPoints + 1);
   double runningLength = 0.0;
   for ( int i=0; i < 100; ++i )
   {
      runningLength += lut[i].first;
      if ( runningLength > targetLength )
      {
         // Pick the closest one
         if ( i > 0 && (targetLength - (runningLength - lut[i-1].first)) < (lut[i].first - targetLength) )
         {
            returnValue.push_back( lut[i-1].second );
            runningLength = targetLength - runningLength + lut[i].first;
         }
         else
         {
            returnValue.push_back( lut[i].second );
            runningLength = runningLength - targetLength;
         }
      }
   }
   

   
//   for ( int i = 1; i <= numInterpolatedPoints; ++i )
//   {
//      double percentage = increment * i;
//      double f[] = { pow(1.0-percentage, 2.0),
//         2.0*percentage*(1.0-percentage),
//         pow(percentage, 2.0) };
//      Eigen::Vector3d p = f[0]*p0 + f[1]*p1 + f[2]*p2;
//      returnValue.push_back( p );
//   }

   return returnValue;
}
Eigen::Quaterniond Utility::Distance( Eigen::Quaterniond & a,
   Eigen::Quaterniond & b )
{
   return a * b.conjugate();
}

// Internal helpers
#ifndef _WIN32
int Utility::FileFilter( const struct dirent * in_directoryEntry )
{
   // Don't include . or ..
   if ( strncmp( in_directoryEntry->d_name, ".", 1 ) == 0 ||
     strncmp( in_directoryEntry->d_name, "..", 2 ) == 0 )
      return 0;
      
   if ( filter )
   {
      if ( regexec( filter, in_directoryEntry->d_name, (size_t) 0, NULL, 0 ) == 0 )
      {
         return 1;
      }
      else
      {
         return 0;
      }
   }
   else
   {
      return 1;
   }
}
#endif
void Utility::BuildRegularExpression( char * inout_regEx, size_t bufLength )
{
   //"(.*\.raw$)"
   char buffer[512];
   int length = std::min( (int)strlen( inout_regEx ), (int)bufLength - 1 );
   int i = 0;
   int j = 0;

   // We'll need parentheses
   buffer[j++] = '(';

   // If the first character is an asterisk
   if ( inout_regEx[0] == '*' )
   {
      // Put a dot in front
      buffer[j++] = '.';
   }
   else
   {
      // Put a caret in front
      buffer[j++] = '^';
   }

   for ( i = 0; i < length; i++ )
   {
      switch( inout_regEx[i] )
      {
         case '.':
         {
            // Escape periods
            buffer[j++] = '\\';
            buffer[j++] = '.';
            break;
         }
         default:
         {
            // Just copy it over
            buffer[j++] = inout_regEx[i];
            break;
         }
      }
   }

   // If the last character is not an asterisk
   if ( i > 0 && inout_regEx[ i - 1 ] != '*' )
   {
      // Put a dollar sign there
      buffer[j++] = '$';
   }

   // Close with parentheses and NULL terminator
   buffer[ j++ ] = ')';
   buffer[ j ] = 0;
   
   // Replace the input with our regular expression
   int maxBytes = std::min(j, (int)bufLength - 1);
   strncpy( inout_regEx, buffer, maxBytes );
   inout_regEx[ maxBytes ] = 0;
}
