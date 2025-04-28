#include "BasicFrame.h"
#include "RVersion.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

#include "TFile.h"
#include "TChain.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"

#include "DataFrameLibrary.h"
#include "CustomMap.h"

// This assumes the options have been set from argc and argv before! That's true when using grsiframe, other programs need to ensure this happens.
BasicFrame::BasicFrame(Options* opt)
	: fOptions(opt)
{
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 24, 0)
   // this increases RDF's verbosity level as long as the `fVerbosity` variable is in scope, i.e. until BasicFrame is destroyed
   if(fOptions->Debug()) {
      fVerbosity = new ROOT::Experimental::RLogScopedVerbosity(ROOT::Detail::RDF::RDFLogChannel(), ROOT::Experimental::ELogLevel::kInfo);
   }
#endif

   std::string treeName = fOptions->TreeName();
	if(treeName.empty()) {
		TFile check(fOptions->InputFiles()[0].c_str());
		if(check.Get("higs_data") != nullptr) {
			treeName = "higs_data";
		}
		check.Close();
	}
	if(treeName.empty()) {
		std::ostringstream str;
      str << "Failed to find 'higs_data' in '" << fOptions->InputFiles()[0] << "', either provide a different tree name via --tree-name flag or check input file" << std::endl;
      throw std::runtime_error(str.str());
   }

   // only enable multi threading if number of threads isn't zero
   if(fOptions->MaxWorkers() > 0) {
      ROOT::EnableImplicitMT(fOptions->MaxWorkers());
   }

   auto* chain = new TChain(treeName.c_str());

   // loop over input files, and add them to the chain
   for(const auto& fileName : fOptions->InputFiles()) {
      if(chain->Add(fileName.c_str(), 0) < 1) {   // setting nentries parameter to zero makes TChain load the file header and return a 1 if the file was opened successfully
         std::cout << "Failed to open '" << fileName << "'" << std::endl;
      }
   }

   std::cout << "Looped over " << chain->GetNtrees() << "/" << fOptions->InputFiles().size() << " files." << std::endl;

   fTotalEntries = chain->GetEntries();

   fDataFrame = new ROOT::RDataFrame(*chain);

   // create an input list to pass to the helper
   auto* inputList = new TList;

	inputList->Add(fOptions->GetCalibration());

   /// Try to load an external library with the correct function in it.
   /// If that library does not exist, try to compile it.
   /// To handle all that we use the class DataFrameLibrary (very similar to TParserLibrary)
   auto* helper  = DataFrameLibrary::Get()->CreateHelper(inputList);
   fOutputPrefix = helper->Prefix();
   // this actually moves the helper to the data frame, so from here on "helper" doesn't refer to the object we created anymore
   // aka don't use helper after this!
   fOutput = helper->Book(fDataFrame);
}

void BasicFrame::Run(Redirect*& redirect)
{
   // get output file name
   std::string outputFileName = Form("%s%s.root", fOutputPrefix.c_str(), fOptions->RunNumberString().c_str());
   std::cout << "Writing to " << outputFileName << std::endl;

   TFile outputFile(outputFileName.c_str(), "recreate");

   // stop redirect before we start the progress bar (storing the files we redirect stdout and stderr to first)
   const auto* outFile = redirect->OutFile();
   const auto* errFile = redirect->ErrFile();

   delete redirect;
   // this is needed so the function that created the redirect know it has ended
   // not really needed right now as we create a new redirect further down, but we have it here in case that gets changed
   redirect = nullptr;

#if ROOT_VERSION_CODE < ROOT_VERSION(6, 30, 0)
   std::string progressBar;
   const auto  barWidth = 100;
   if(!fOptions->Debug()) {
      // create a progress bar with percentage
      auto       entries = fDataFrame->Count();
      std::mutex barMutex;   // Only one thread at a time can lock a mutex. Let's use this to avoid concurrent printing.
      const auto everyN = fTotalEntries / barWidth;
      entries.OnPartialResultSlot(everyN, [&everyN, &fTotalEntries = fTotalEntries, &progressBar, &barMutex](unsigned int /*slot*/, ULong64_t& /*partialList*/) {
         std::lock_guard<std::mutex> lock(barMutex);   // lock_guard locks the mutex at construction, releases it at destruction
         static int                  counter = 1;
         progressBar.push_back('#');
         // re-print the line with the progress bar
         std::cout << "\r[" << std::left << std::setw(barWidth) << progressBar << ' ' << std::setw(3) << (counter * everyN * 100) / fTotalEntries << " %]" << std::flush;
         ++counter;
      });
   }
#else
   ROOT::RDF::Experimental::AddProgressBar(*fDataFrame);
#endif

   if(fOutput != nullptr) {
      // accessing the result from Book causes the actual processing of the helper
      // so we try and catch any exception
      try {
         for(auto& list : *fOutput) {
            // try and switch to the directory this list should be written to
            if(gDirectory->GetDirectory(list.first.c_str()) && gDirectory->cd(list.first.c_str())) {
               list.second.Write();
            } else {
               // directory this list should be written to doesn't exist, so create it
               gDirectory->mkdir(list.first.c_str());
               if(gDirectory->cd(list.first.c_str())) {
                  list.second.Write();
               } else {
                  std::cout << "Error, failed to find or create path " << list.first << ", writing into " << gDirectory->GetPath() << std::endl;
               }
            }
            // switch back to topmost directory
            while(gDirectory->GetDirectory("..")) { gDirectory->cd(".."); }
         }
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 30, 0)
         std::cout << "\r[" << std::left << std::setw(barWidth) << progressBar << ' ' << "100 %]" << std::flush;
#endif
      } catch(CustomMapException<std::string>& e) {
         std::cout << DRED << "Exception in " << __PRETTY_FUNCTION__ << ": " << e.detail() << RESET_COLOR << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
         throw e;
      }
   } else {
      std::cout << "Error, output list is nullptr!" << std::endl;
   }

	fOptions->GetCalibration()->Write();

   // start new redirect, appending to the previous files we had redirected to
   redirect = new Redirect(outFile, errFile, true);

   outputFile.Close();
   std::cout << "Closed '" << outputFile.GetName() << "'" << std::endl;
}

void DummyFunctionToLocateBasicFrameLibrary()
{
   // does nothing
}
