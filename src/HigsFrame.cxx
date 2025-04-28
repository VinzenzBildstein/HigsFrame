#include <iostream>
#include <string>
#include <vector>

#include "RVersion.h"
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 14, 0)
#include "TStyle.h"
#include "TFile.h"
#include "TChain.h"
#include "ROOT/RDataFrame.hxx"
#include "TEnv.h"
#include "TSystem.h"
#include "TStopwatch.h"

#include "Options.h"
#include "Redirect.h"
#include "BasicFrame.h"

int main(int argc, char** argv)
{
   auto* stopwatch = new TStopwatch;

	auto* options = Options::Get();

   // parse input options
	bool parseError = false;
   for(int i = 1; i < argc; ++i) {
		if(strcmp(argv[i], "--input") == 0 || strcmp(argv[i], "-i") == 0) {
			while(i + 1 < argc && argv[i + 1][0] != '-') {
				options->AddInputFile(argv[++i]);
			}
			continue;
		}
		if(strcmp(argv[i], "--output") == 0 || strcmp(argv[i], "-o") == 0) {
			options->OutputFileName(argv[++i]);
			continue;
		}
		if(strcmp(argv[i], "--helper") == 0 || strcmp(argv[i], "-h") == 0) {
			options->Helper(argv[++i]);
			continue;
		}
		if(strcmp(argv[i], "--calibration") == 0 || strcmp(argv[i], "-c") == 0) {
			options->SetCalibration(argv[++i]);
			continue;
		}
		if(strcmp(argv[i], "--tree-name") == 0 || strcmp(argv[i], "-t") == 0) {
			options->TreeName(argv[++i]);
			continue;
		}
		if(strcmp(argv[i], "--max-workers") == 0 || strcmp(argv[i], "-w") == 0) {
			options->MaxWorkers(std::stoi(argv[++i]));
			continue;
		}
		if(strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
			options->Debug(true);
			continue;
		}
		std::cout << "Unkown command line option \"" << argv[i] << "\":" << std::endl;
		parseError = true;
   }

   // check that we have an input file and a helper to run on it
   if(options->InputFiles().empty()) {
      std::cerr << "No input files provided!" << std::endl;
      parseError = true;
   }
   if(options->Helper().empty()) {
      std::cerr << "No datahelper source (*.cxx file) provided!" << std::endl;
      parseError = true;
   }

	if(parseError) {
		std::cout << "Commandline arguments for " << argv[0] << ":" << std::endl
					 << "--input        <input root-file>                        needed" << std::endl
					 << "--helper       <datahelper source file>                 needed" << std::endl
					 << "--calibration  <calibration file>                       optional" << std::endl
					 << "--max-workers  <maximum number of threads>              optional" << std::endl
					 << "--output       <output root-file>                       optional" << std::endl
					 << "--tree-name    <name of root tree>                      optional" << std::endl
					 << "--debug        no argument, enables debugging messages  optional" << std::endl;
		return 1;
	}

	if(options->Debug()) {
		options->Print();
	}

	// check the input file name and see if we can determine the run number
	// assuming the name is xxxx_run???.bin_tree.root
	std::string runNumberString = options->RunNumberString();
	if(options->Debug()) {
		std::cout << "Got run number string " << runNumberString << std::endl;
	}

   // determine the name of the helper (from the provided helper library) to create a redirect of stdout
   std::string logFileName = options->Helper();
   logFileName             = logFileName.substr(logFileName.find_last_of('/') + 1);   // strip everything before the last slash
   if(logFileName.find("Helper") != std::string::npos) {
      logFileName = logFileName.substr(0, logFileName.find("Helper"));   // strip "Helper" and anything after it (like the extension)
   } else {
      logFileName = logFileName.substr(0, logFileName.find_last_of('.'));   // strip extension since we didn't find "Helper" in the name
   }
   logFileName.append(runNumberString);
   logFileName.append(".log");

   // start redirect of stdout only w/o appending (ends when we delete it)
   std::cout << "redirecting stdout to " << logFileName << std::endl;
   auto* redirect = new Redirect(logFileName.c_str(), nullptr, false);

   // this reads and compiles the user code
   BasicFrame frame(options);
   // run it and write the results
   frame.Run(redirect);

   // re-start redirect of stdout only w/ appending if needed (ends when we delete it)
   if(redirect == nullptr) {
      redirect = new Redirect(logFileName.c_str(), nullptr, true);
   }

   // print time it took to run
   double realTime = stopwatch->RealTime();
   int    hour     = static_cast<int>(realTime / 3600);
   realTime -= hour * 3600;
   int min = static_cast<int>(realTime / 60);
   realTime -= min * 60;
   // print goes to log file due to redirect, so we don't need colours here
   std::cout << std::endl
             << "Done after " << hour << ":" << std::setfill('0') << std::setw(2) << min << ":"
             << std::setprecision(3) << std::fixed << realTime << " h:m:s"
             << std::endl;

   // delete the redirect and print again to true stdout
   delete redirect;

   std::cout << DMAGENTA << std::endl
             << "Done after " << hour << ":" << std::setfill('0') << std::setw(2) << min << ":"
             << std::setprecision(3) << std::fixed << realTime << " h:m:s"
             << RESET_COLOR << std::endl;

   return 0;
}
#else
int main(int, char** argv)
{
   std::cerr << argv[0] << ": need at least ROOT version 6.14" << std::endl;
   return 1;
}
#endif
