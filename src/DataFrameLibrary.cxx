#include "DataFrameLibrary.h"
#include "RVersion.h"

#define dlsym __bull__
#include <dlfcn.h>
#undef dlsym

#include <sstream>
#include <cstdlib>
#include <sys/stat.h>

#include "Options.h"
#include "BasicFrame.h"

bool FileExists(const char* filename)
{
   /// This checks if the path exist, and if it is a file and not a directory!
   struct stat buffer{};
   int         state = stat(filename, &buffer);
   // state != 0 means we couldn't get file attributes. This doesn't necessary mean the file
   // does not exist, we might just be missing permission to access it. But for our purposes
   // this is the same as the file not existing.
   if(state != 0) { return false; }
   // we got the file attributes, so it exsist, we just need to check if it is a directory.
   return !S_ISDIR(buffer.st_mode);
}


// redeclare dlsym to be a function returning a function pointer instead of void *
extern "C" void* (*dlsym(void* handle, const char* symbol))();

DataFrameLibrary::~DataFrameLibrary()
{
   if(fHandle != nullptr) {
      dlclose(fHandle);
   }
}

void DataFrameLibrary::Load()
{
   if(fHandle != nullptr) {
      std::cout << "Already loaded handle " << fHandle << std::endl;
      return;
   }

   std::string libraryPath = Options::Get()->Helper();
   if(libraryPath.empty()) {
      std::ostringstream str;
      str << DRED << "No data frame library provided! Please provided the location of the data frame library on the command line." << RESET_COLOR;
      throw std::runtime_error(str.str());
   }

   // check if the provided file ends in .cxx (which means we should compile it into a library) otherwise we just try to load it
   // if there is no dot in the path or the dot is before a slash in the path we just try to load this weirdly named library
   size_t dot   = libraryPath.find_last_of('.');
   size_t slash = libraryPath.find_last_of('/');
   if(dot != std::string::npos && (dot > slash || slash == std::string::npos) && libraryPath.substr(dot) == ".cxx") {
      // let's get the full path first (or maybe move this into the function?)
      Compile(libraryPath, dot, slash);
      // replace the .cxx extension with .so
      libraryPath.replace(dot, std::string::npos, ".so");
   }

   if(!FileExists(libraryPath.c_str())) {
      std::ostringstream str;
      str << DRED << "Library '" << libraryPath << "' does not exist or we do not have permissions to access it!" << RESET_COLOR;
      throw std::runtime_error(str.str());
   }

   fHandle = dlopen(libraryPath.c_str(), RTLD_LAZY);
   if(fHandle == nullptr) {
      std::ostringstream str;
      str << DRED << "Failed to open data frame library '" << libraryPath << "': " << dlerror() << "!" << RESET_COLOR;
      std::cout << "dlerror: '" << dlerror() << "'" << std::endl;
      throw std::runtime_error(str.str());
   }
   // try and get constructor and destructor functions from opened library
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
   fCreateHelper  = reinterpret_cast<BasicHelper* (*)(TList*)>(dlsym(fHandle, "CreateHelper"));
   fDestroyHelper = reinterpret_cast<void (*)(BasicHelper*)>(dlsym(fHandle, "DestroyHelper"));
#pragma GCC diagnostic pop

   if(fCreateHelper == nullptr || fDestroyHelper == nullptr) {
      std::ostringstream str;
      str << DRED << "Failed to find CreateHelper, and/or DestroyHelper functions in library '" << libraryPath << "'!" << RESET_COLOR;
      throw std::runtime_error(str.str());
   }
   std::cout << "\tUsing library " << libraryPath << std::endl;
}

void DataFrameLibrary::Compile(std::string& path, const size_t& dot, const size_t& slash)
{
   /// \brief
   /// Try and compile the provided .cxx file into a shared object library using the provided
   /// path, position of the last dot, and the position of the last slash.
   /// \details
   /// Other flags used are "-c -fPIC -g", `root-config --cflags --glibs`, and the directory
   /// the path points to as include directory.
   /// \param[in] path path of the .cxx file
   /// \param[in] dot position of the last dot (guaranteed to be after the last slash!)
   /// \param[in] slash position of the last slash (can be std::string::npos)

   // first we create the paths for the header file, and shared library
   // we know dot != npos and either dot > slash or slash = npos
   std::string sourceFile    = path;
   std::string headerFile    = path.replace(dot, std::string::npos, ".hh");
   std::string sharedLibrary = path.replace(dot, std::string::npos, ".so");
   // first we get the stats of the file's involved (.cxx, .hh, .so)
   struct stat sourceStat{};
   if(stat(sourceFile.c_str(), &sourceStat) != 0) {
      std::ostringstream str;
      str << "Unable to access stat of source file " << sourceFile << std::endl;
      throw std::runtime_error(str.str());
   }
   struct stat headerStat{};
   if(stat(headerFile.c_str(), &headerStat) != 0) {
      std::ostringstream str;
      str << "Unable to access stat of header file " << headerFile << std::endl;
      throw std::runtime_error(str.str());
   }
   struct stat frameLibStat{};
   // get path of libTGRSIFrame via
   Dl_info info;
   if(dladdr(reinterpret_cast<void*>(DummyFunctionToLocateBasicFrameLibrary), &info) == 0) {
      std::ostringstream str;
      str << "Unable to find location of DummyFunctionToLocateBasicFrameLibrary" << std::endl;
      throw std::runtime_error(str.str());
   }
   if(stat(info.dli_fname, &frameLibStat) != 0) {
      std::ostringstream str;
      str << "Unable to access stat of " << info.dli_fname << std::endl;
      throw std::runtime_error(str.str());
   }
   struct stat sharedLibStat{};
   if(stat(sharedLibrary.c_str(), &sharedLibStat) == 0 &&
      sharedLibStat.st_atime > sourceStat.st_atime &&
      sharedLibStat.st_atime > headerStat.st_atime &&
      sharedLibStat.st_atime > frameLibStat.st_atime) {
      std::cout << DCYAN << "shared library " << sharedLibrary << " exists and is newer than " << sourceFile << ", " << headerFile << ", and libBasicFrame.so" << RESET_COLOR << std::endl;
      return;
   }
   // get include path
   std::string includePath = ".";
   if(slash != std::string::npos) {
      includePath = path.substr(0, slash);
   }
   std::cout << DCYAN << "----------  starting compilation of user code  ----------" << RESET_COLOR << std::endl;
   std::string        objectFile        = path.replace(dot, std::string::npos, ".o");
   std::ostringstream command;
   command << "g++ -c -fPIC -g $(root-config --cflags) -I" << includePath;
#ifdef OS_DARWIN
   command << " -I/opt/local/include ";
#endif
   command << " -o " << objectFile << " " << sourceFile;
   if(std::system(command.str().c_str()) != 0) {
      std::ostringstream str;
      str << "Unable to compile source file " << sourceFile << " using " << DBLUE << "'" << command.str() << "'" << RESET_COLOR << std::endl;
      throw std::runtime_error(str.str());
   }
   std::cout << DCYAN << "----------  starting linking user code  -----------------" << RESET_COLOR << std::endl;
   std::ostringstream().swap(command);   // create new (empty) stringstream and swap it with command this resets the underlying string and all error flags
   command << "g++ -fPIC -g -shared $(root-config --glibs) -o " << sharedLibrary << " " << objectFile;
   if(std::system(command.str().c_str()) != 0) {
      std::ostringstream str;
      str << "Unable to link shared object library " << sharedLibrary << " using " << DBLUE << "'" << command.str() << "'" << RESET_COLOR << std::endl;
      throw std::runtime_error(str.str());
   }
   std::cout << DCYAN << "----------  done compiling user code  -------------------" << RESET_COLOR << std::endl;
}
