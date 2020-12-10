#include <stdlib.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <signal.h>
#include <map>
#include <sys/types.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include "config.h"
#include "Utility.hpp"
#include "Animation.hpp"
#include "PoseFactory.hpp"
#include "KpImporterFactory.hpp"
#include "RigPose.hpp"
#include "CLI11.hpp"

#ifdef _WIN32
#else
   #include <unistd.h>
   #include <sys/wait.h>
#endif

bool quit = false;
struct Args
{
   std::string inputFolder;
   std::vector< std::string > inputFiles;
   std::string outputDirectory = ".";
   double segmentDuration = 0;
   double fps = 30.0;
   double unitMeterNorm = 1.;
   std::string smooth = "lpf_ipp";
   double maxGap = 0.5;
   bool useLeftHandCoords = false;
   bool stream = false;
   bool printVersion = false;
} args;

#ifndef STDIN
   #define STDIN 0
#endif

void signalHandler( int signal )
{
   if (signal == SIGINT)
   {
      quit = true;
#ifndef _WIN32
      close( STDIN );
#endif
   }
}
void ParseFilesOrDirectory( const std::vector< std::string > & filesOrDirectory )
{
   // Base everytihng on the first element
   if ( Utility::IsDirectory( filesOrDirectory[0] ) )
   {
      args.inputFolder = filesOrDirectory[0];
      args.inputFiles = Utility::GetFiles( args.inputFolder, "" );
   }
   else
   {
      args.inputFiles = filesOrDirectory;
   }
}

int main( int argc, char *argv[] )
{
   std::unique_ptr< Pose > characterPose;
   int totalMissingFrames = 0;
   int totalDuplicateFrames = 0;
   int lowestTimestamp = INT_MAX;
   int highestTimestamp = 0;
   bool suppressError = false;
   KpImporter::IMPORT_TYPE importerType = KpImporter::IMPORT_TYPE_UNKNOWN;
   int fileIndex = 0;
   
   struct CharacterMetadata
   {
      int contiguousMissingFrames = 0;
      int previousTimestamp = -1;
   };
   std::map< std::string, CharacterMetadata > characterMetadata;
   
   // Set up our signal handler
   signal( SIGINT, signalHandler );
   
   // Process command-line arguments
   CLI::App app{ "App description" };
   
   app.add_flag( "--version", args.printVersion, "Prints the version string" );
   app.add_flag( "-l,--left", args.useLeftHandCoords, "Output a left-handed coordinate system. Default is right\n");
   auto * streamOption = app.add_flag( "-s,--stream", args.stream, "Read from STDIN instead of files. This is useful for live streaming\n" );
   app.add_option( "--max-gap", args.maxGap, "Maximum gap, in seconds, of missing frames to interpolate. Gaps larger than this will not interpolate but instead copy/paste the previous frame, resulting in a \"freeze\". Default is 0.5\n" );
   app.add_option( "--smooth", args.smooth, "Specify the smoothing algorithm {none|lpf_ipp}. Defaul is lpf_ipp\n" );
   app.add_option( "-o,--outdir", args.outputDirectory, "Set the output directory for the rig file (or rig file segments). Default is the working directory\n" );
   app.add_option( "-r,--rate", args.fps, "Frames-per-second (fps). Default is 30\n" );
   app.add_option( "--segsize", args.segmentDuration, "Segment duration in seconds. Default is 0, meaning output a monolithic file\n" );
   app.add_option( "-u,--units", args.unitMeterNorm, "\"Normalization\" value used to convert input units to meters; E.g., if your input data uses units of decimeters then you would pass in a value of 0.1. Default is 1.0 (meters)\n" );
   
   // The default arguments are files or a directory
   std::vector< std::string > filesOrDirectory;
   app.add_option_function< std::vector< std::string > >( "files-or-directory", ParseFilesOrDirectory, "Input files or directory" )->excludes( streamOption );

   CLI11_PARSE( app, argc, argv );
   
   // If --version then ignore everything else, print the version, and exit
   if ( args.printVersion )
   {
      std::cout << "kp2rig " << MY_VERSION << std::endl;
      return 0;
   }
   
   // We require at least a file/directory or the streaming option
   if ( args.inputFiles.size() == 0 &&
      args.inputFolder.size() == 0 &&
      args.stream == false )
   {
      std::cout << app.help();
      exit( 0 );
   }
   
   // Command line parsed, now use them
   Animation animation( args.fps );
   animation.SegmentDuration( args.segmentDuration );
   animation.OutputDirectory( args.outputDirectory );
   animation.Smooth( SmoothFactory::SmoothType( args.smooth ) );
   animation.MaxMissingFrameGap( args.maxGap );
   
   // Print a message if capturing from stdin
   if ( args.stream )
   {
      printf( "capturing from STDIN...\n" );
   }
   
   // Until told to quit
   while ( !quit )
   {
      std::string filename;
      if ( args.inputFiles.size() )
      {
         // Stop when we run out of files
         if ( fileIndex >= (int)args.inputFiles.size() )
         {
            quit = true;
            break;
         }
         
         // Get the filename with path
         std::stringstream ss;
         if ( args.inputFolder != "" )
            ss << args.inputFolder << "/";
         ss << args.inputFiles[ fileIndex++ ];
         
         // Determine which type of parser we need for this file
         importerType = KpImporterFactory::DetermineType( ss.str() );
         filename = ss.str();
      }
      else
      {
         importerType = KpImporter::IMPORT_TYPE_CSV_STREAM;
         filename = "";
      }
      
      std::unique_ptr< KpImporter > importer;
      try
      {
         // Create our importer
         importer = KpImporterFactory::Create( importerType );
         
         // Set the units
         importer->UnitMeterNorm( args.unitMeterNorm );
         
         // Open the file
         importer->Open( filename );
      }
      catch ( std::runtime_error e )
      {
         std::cerr << e.what() << std::endl;
         continue;
      }

      // Process all the data
      while ( !importer->IsParseComplete() )
      {
         try
         {
            characterPose = importer->ReadOne();
            suppressError = false;
         }
         catch( std::runtime_error e )
         {
            // If this is not the end of the file
            if ( !importer->IsParseComplete() )
            {
               if ( !suppressError )
                  std::cerr << e.what() << std::endl;
               suppressError = true;
            }
            
            break;
         }
         
         if ( characterPose )
         {
            // Get this character's metadata
            CharacterMetadata & metadata = characterMetadata[characterPose->Name()];
            
            // Set our axis scalars
            characterPose->CoordinateSystem( { 1., 1., args.useLeftHandCoords ? -1. : 1. } );
            
            // Keep track of timestamps, looking for negative or missing frames
            if ( metadata.previousTimestamp < 0 )
            {
               metadata.previousTimestamp = characterPose->Timestamp() - 1;
            }
            
            if ( metadata.previousTimestamp == characterPose->Timestamp() )
            {
               printf( "%s: Duplicate frame %d dropped\n", characterPose->Name().c_str(), metadata.previousTimestamp );
               ++totalDuplicateFrames;
            }
            else
            {
               if ( characterPose->Timestamp() - metadata.previousTimestamp > 1 )
               {
                  metadata.contiguousMissingFrames = characterPose->Timestamp() - (metadata.previousTimestamp + 1);
                  if ( metadata.contiguousMissingFrames == 1 )
                     printf( "%s: Missing frame %d\n", characterPose->Name().c_str(), metadata.previousTimestamp + 1 );
                  else
                     printf( "%s: Missing %d frames, %d-%d expected\n",
                        characterPose->Name().c_str(),
                        metadata.contiguousMissingFrames,
                        metadata.previousTimestamp + 1,
                        metadata.previousTimestamp + metadata.contiguousMissingFrames );
                        
                  totalMissingFrames += metadata.contiguousMissingFrames;
               }

               metadata.previousTimestamp = characterPose->Timestamp();
               metadata.previousTimestamp < lowestTimestamp ? lowestTimestamp = metadata.previousTimestamp : lowestTimestamp;
               metadata.previousTimestamp > highestTimestamp ? highestTimestamp = metadata.previousTimestamp : highestTimestamp;

               try
               {
                  animation.AddPose( characterPose->Name(), characterPose );
               }
               catch( std::runtime_error e )
               {
                  printf( "%s\n", e.what() );
               }
            }
         }
      }
   }

   // Since we're done processing input, make sure all processing is complete
   // before we print our summary and exit
   animation.FlushSegments();
   
   // For printing-sake, make sure start timestamp isn't greater than end timestamp
   if ( lowestTimestamp > highestTimestamp )
      lowestTimestamp = highestTimestamp;
   
   // Print a summary
   const std::vector< std::string > segmentFilenames = animation.SegmentFilenames();
   if ( segmentFilenames.size() )
   {
      printf( "%c", '\n' );
      printf( "------------------------------------------------------\n" );
      printf( "%d characters, bounds %d->%d\n",
         (int)animation.GetAnimatedRigs().size(),
         lowestTimestamp,
         highestTimestamp );
      printf( "%d missing frames, %d duplicate frames\n",
         totalMissingFrames,
         totalDuplicateFrames );
      if ( segmentFilenames.size() == 1 )
      {
         printf( "Output to file '%s'\n",
            segmentFilenames[ 0 ].c_str() );
      }
      else
      {
         printf( "Output %d file segments, starting with '%s'\n",
            (int)segmentFilenames.size(),
            segmentFilenames[ 0 ].c_str() );
      }
   }
   return 0;
}
