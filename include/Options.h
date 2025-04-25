#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include <vector>

#include "Singleton.h"
#include "Calibration.h"

class Calibration;

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

	Calibration* Calibration() const { return fCalibration; }

	// setters
	void Debug(bool debug) { fDebug = debug; if(fDebug) { std::cout << "Debugging enabled!" << std::endl; } }

	void AddInputFile(const char* file) { fInputFiles.emplace_back(file); UpdateRunNumberString(); }

	void TreeName(const char* name) { fTreeName = name; }

	void MaxWorkers(int num) { fMaxWorkers = std::move(num); }

	void OutputFileName(const char* file) { fOutputFileName = file; }

	void Helper(const char* source) { fHelper = source; }

	void Calibration(const char* file) { delete fCalibration; fCalibration = new class Calibration(file); }

	void Print()
	{
		std::cout << "Debugging is" << (fDebug ? " " : " not ") << "enabled" << std::endl;
		std::cout << "Got " << fInputFiles.size() << " input files:" << std::endl;
		for(auto& file : fInputFiles) {
			std::cout << file << std::endl;
		}
		std::cout << "Using tree name " << (fTreeName.empty() ? "higsdata (default)" : fTreeName) << std::endl;
		std::cout << "Running on " << fMaxWorkers << " workers" << std::endl;
		std::cout << "Got a run number string \"" << fRunNumberString << "\"" << std::endl;
		std::cout << "Using helper " << fHelper << std::endl;
	}

private:
	void UpdateRunNumberString() {
		fRunNumberString.erase();
		if(fInputFiles.empty()) {
			if(fDebug) {
				std::cout << "No input files => empty run number string" << std::endl;
			}
			return;
		}
		// add the run number of the first input file to the run number string
		// remove everything after the penultimate dot
		std::string tmpString = fInputFiles[0].substr(0, fInputFiles[0].find_last_of('.', fInputFiles[0].find_last_of('.') - 1));
		if(fDebug) {
			std::cout << "Using first input file \"" << fInputFiles[0] << "\" got \"" << tmpString << "\"" << std::endl;
		}
		// keep only last three characters
		fRunNumberString = tmpString.substr(tmpString.length()-3, 3);
		if(fDebug) {
			std::cout << "Current run number string is \"" << fRunNumberString << "\"" << std::endl;
		}
		// if we have more than one run, we add the run number of the last input file
		if(fInputFiles.size() > 1) {
			fRunNumberString.append("-");
			tmpString = fInputFiles.back().substr(0, fInputFiles.back().find_last_of('.', fInputFiles.back().find_last_of('.') - 1));
			if(fDebug) {
				std::cout << "Using last input file \"" << fInputFiles.back() << "\" got \"" << tmpString << "\"" << std::endl;
			}
			fRunNumberString.append(tmpString.substr(tmpString.length()-3, 3));
			if(fDebug) {
				std::cout << "Updated run number string is \"" << fRunNumberString << "\"" << std::endl;
			}
		}
	}

	bool fDebug{false};
	std::vector<std::string> fInputFiles;
	std::string fOutputFileName;
	std::string fTreeName;
	std::string fRunNumberString;
	std::string fHelper;
	int fMaxWorkers{0};
	class Calibration* fCalibration{nullptr};
};
#endif

