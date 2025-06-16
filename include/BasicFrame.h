#ifndef BASICFRAME_H
#define BASICFRAME_H
#include "RVersion.h"

#include <map>
#include <string>

#include "TList.h"

#include "ROOT/RDataFrame.hxx"
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 24, 0)
#include "ROOT/RLogger.hxx"
#endif

#include "Redirect.h"
#include "Options.h"

class BasicFrame {
public:
   explicit BasicFrame(Options* opt);

   void Run(Redirect*& redirect);

private:
   Options*                                            fOptions;
   std::string                                         fOutputPrefix{"default"};
   ROOT::RDF::RResultPtr<std::map<std::string, TList>> fOutput;

   ROOT::RDataFrame* fDataFrame{nullptr};
   Long64_t          fTotalEntries{0};

#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 24, 0)
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 36, 0)
    ROOT::RLogScopedVerbosity* fVerbosity{nullptr};
#else
   ROOT::Experimental::RLogScopedVerbosity* fVerbosity{nullptr};
#endif
#endif
};

/*! @} */
void DummyFunctionToLocateBasicFrameLibrary();

#endif
