#ifndef TDATAFRAMELIBRARY_H
#define TDATAFRAMELIBRARY_H
#include "RVersion.h"

#include <string>

#include "TList.h"

#include "Singleton.h"
#include "BasicHelper.h"

class DataFrameLibrary : public Singleton<DataFrameLibrary> {
public:
   friend class Singleton<DataFrameLibrary>;

   ~DataFrameLibrary();

   void Load();   ///< if necessary loads shared object library and sets/initializes all other functions

   BasicHelper* CreateHelper(TList* list)
   {
      /// function to open library specific data parser
      Load();
      return fCreateHelper(list);
   }
   void DestroyHelper(BasicHelper* parser)
   {
      /// function to destroy library specific data parser
      Load();
      fDestroyHelper(parser);
   }

private:
   DataFrameLibrary()                                   = default;
   DataFrameLibrary(const DataFrameLibrary&)            = default;
   DataFrameLibrary(DataFrameLibrary&&)                 = default;
   DataFrameLibrary& operator=(const DataFrameLibrary&) = default;
   DataFrameLibrary& operator=(DataFrameLibrary&&)      = default;

   static void Compile(std::string& path, const size_t& dot, const size_t& slash);

   void* fHandle{nullptr};   ///< handle for shared object library

   BasicHelper* (*fCreateHelper)(TList*);
   void (*fDestroyHelper)(BasicHelper*);

   /// \cond CLASSIMP
   ClassDefOverride(DataFrameLibrary, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};

#endif
