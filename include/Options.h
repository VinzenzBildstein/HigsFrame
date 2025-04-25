#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include <vector>

#include "Singleton.h"

class Options : public Singleton<Options> {
public:
	Options() = default;

	// getters
	bool Debug() const { return fDebug; }

	std::vector<std::string> InputFiles() const { return fInputFiles; }

	std::string TreeName() const { return fTreeName; }

	int MaxWorkers() const { return fMaxWorkers; }

	std::string RunNumberString() const { return fRunNumberString; }

	std::string Helper() const { return fHelper; }

	// setters
	void AddInputFile(const char* file) { fInputFiles.emplace_back(file); UpdateRunNumberString(); }

	void OutputFileName(const char* file) { fOutputFileName = file; }

	void Helper(const char* source) { fHelper = source; }

	void Debug(bool debug) { fDebug = debug; }

private:
	void UpdateRunNumberString() {
		fRunNumberString.erase();
		if(fInputFiles.empty()) {
			return;
		}
		// add the run number of the first input file to the run number string
		// remove everything after the penultimate dot
		std::string tmpString = fInputFiles[0].substr(0, fInputFiles[0].find_last_of('.', fInputFiles[0].find_last_of('.') - 1) - 1);
		// keep only last three characters
		fRunNumberString = tmpString.substr(tmpString.length()-3, 3);
		// if we have more than one run, we add the run number of the last input file
		if(fInputFiles.size() > 1) {
			fRunNumberString.append("-");
			tmpString = fInputFiles.back().substr(0, fInputFiles.back().find_last_of('.', fInputFiles.back().find_last_of('.') - 1) - 1);
			fRunNumberString.append(tmpString.substr(tmpString.length()-3, 3));
		}
	}

	bool fDebug{false};
	std::vector<std::string> fInputFiles;
	std::string fOutputFileName;
	std::string fTreeName;
	std::string fRunNumberString;
	std::string fHelper;
	int fMaxWorkers;
};
#endif

