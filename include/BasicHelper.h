#ifndef TGRSIHELPER_H
#define TGRSIHELPER_H
#include "RVersion.h"
#include "ROOT/RDataFrame.hxx"
#include "TObject.h"
#include "TList.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TTree.h"
#include "TCutG.h"
#include "TBufferFile.h"

#include "Options.h"
#include "CustomMap.h"

////////////////////////////////////////////////////////////////////////////////
///
/// \class BasicHelper
///
/// Base class for all helpers used in HigsFrame.
/// It provides some general members that are set from the input list.
/// It also loads settings from the input list.
///
////////////////////////////////////////////////////////////////////////////////

class BasicHelper : public TObject {
public:
   std::string& Prefix() { return fPrefix; }

protected:
   std::vector<std::shared_ptr<std::map<std::string, TList>>> fLists;                   // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! one map of lists and directories per data processing slot to hold all output objects
   std::vector<CustomMap<std::string, TH1*>>                   fH1;                      // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! one map per data processing slot for 1D histograms
   std::vector<CustomMap<std::string, TH2*>>                   fH2;                      // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! one map per data processing slot for 2D histograms
   std::vector<CustomMap<std::string, TH3*>>                   fH3;                      // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! one map per data processing slot for 3D histograms
   std::vector<CustomMap<std::string, TTree*>>                 fTree;                    // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! one map per data processing slot for trees
   std::vector<CustomMap<std::string, TObject*>>               fObject;                  // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! one map per data processing slot for any TObjects
   std::map<std::string, TCutG*>                              fCuts;                    // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! map of cuts
   std::string                                                fPrefix{"BasicHelper"};   // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes) //!<! name of this action (used as prefix)

private:
   static constexpr int fSizeLimit = 1073741822;   //!<! 1 GiB size limit for objects in ROOT
   void                 CheckSizes(unsigned int slot, const char* usage);

public:
   /// This type is a requirement for every helper.
   using Result_t = std::map<std::string, TList>;

   explicit BasicHelper(TList* input);

   /// This function builds the vectors of TLists and maps for 1D- and 2D-histograms.
   /// It calls the overloaded CreateHistograms functions in which the user can define
   /// their histograms. Then it adds all those histograms to the list of the corresponding slot.
   virtual void Setup();
   /// Virtual helper function that the user uses to create their histograms
   virtual void CreateHistograms(unsigned int)
   {
      std::cout << this << " - " << __PRETTY_FUNCTION__ << ", " << Prefix() << ": This function should not get called, the user's code should replace it. Not creating any histograms!" << std::endl;   // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   }
   /// This method will call the Book action on the provided dataframe
   virtual ROOT::RDF::RResultPtr<std::map<std::string, TList>> Book(ROOT::RDataFrame*)
   {
      std::cout << this << " - " << __PRETTY_FUNCTION__ << ", " << Prefix() << ": This function should not get called, the user's code should replace it. Returning empty list!" << std::endl;   // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      return {};
   }

   BasicHelper(const BasicHelper&)            = delete;
   BasicHelper(BasicHelper&&)                 = default;
   BasicHelper& operator=(const BasicHelper&) = delete;
   BasicHelper& operator=(BasicHelper&&)      = default;
   ~BasicHelper()                             = default;
   std::shared_ptr<std::map<std::string, TList>> GetResultPtr() const { return fLists[0]; }
   void                                          InitTask(TTreeReader*, unsigned int) {}
   void                                          Initialize() {}   // required method, gets called once before starting the event loop
   /// This required method is called at the end of the event loop. It is used to merge all the internal TLists which
   /// were used in each of the data processing slots.
   void Finalize();

   /// This method gets called at the end of Finalize()
   virtual void EndOfSort(std::shared_ptr<std::map<std::string, TList>>&) {}

   std::string Prefix() const { return fPrefix; }
   void        Prefix(const std::string& val) { fPrefix = val; }
   std::string GetActionName() const { return Prefix(); }   // apparently a required function (not documented but doesn't compile w/o it)
};

#endif
