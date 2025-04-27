#include "ExampleHelper.hh"

void ExampleHelper::CreateHistograms(unsigned int slot)
{
   // some variables to easily change range and binning for multiple histograms at once
   int    energyBins = 10000;
   double lowEnergy  = 0.;
   double highEnergy = 2000.;

	// single energy spectra
   fH1[slot]["crossE"] = new TH1F("crossE", Form("Cross energy;energy [keV];counts/%.1f keV", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);
   fH1[slot]["backE"] = new TH1F("backE", Form("Back energy;energy [keV];counts/%.1f keV", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);
   fH1[slot]["miscE"] = new TH1F("miscE", Form("Misc energy;energy [keV];counts/%.1f keV", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);
   fH1[slot]["cebrCh"] = new TH1F("cebrCh", Form("CeBr channel;channel;counts/%.1f channel", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);

	fH1[slot]["crossAddbackE"] = new TH1F("crossAddbackE", Form("Cross energy using addback;energy [keV];counts/%.1f keV", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);

	// timing spectra
   fH2[slot]["crossT"] = new TH2F("crossT", "Cross ID vs timinig relative to Cross_{0};time [ns];Cross ID", 1000, -50000., 50000., 15, 0.5, 15.5);

	// hit pattern spectrum
	fH2[slot]["hp"] = new TH2F("hp", "Hit pattern (cross = 0-15, back = 16-31, misc = 32-47, cebr = 48-63)", 64, -0.5, 63.5, 64, -0.5, 63.5);
}

// TODO: Change the function arguments to match the detectors you want to use and the declaration in the header file!
void ExampleHelper::Exec(unsigned int slot, ROOT::RVecD& crossAmplitude, ROOT::RVecD& crossChannelTime, ROOT::RVecD& crossModuleTS, ROOT::RVecD& crossPileup, ROOT::RVecD& crossTriggerTime, ROOT::RVecD& extendedTS, ROOT::RVecD& backAmplitude, ROOT::RVecD& backChannelTime, ROOT::RVecD& backModuleTS, ROOT::RVecD& backPileup, ROOT::RVecD& backTriggerTime, ROOT::RVecD& miscAmplitude, ROOT::RVecD& miscChannelTime, ROOT::RVecD& miscModuleTS, ROOT::RVecD& miscPileup, ROOT::RVecD& miscTriggerTime, ROOT::RVecD& cebrChannelTime, ROOT::RVecD& cebrIntLong, ROOT::RVecD& cebrModuleTS, ROOT::RVecD& cebrTriggerTime)
{
   // we use .at() here instead of [] so that we get meaningful error message if a histogram we try to fill wasn't created
   // e.g. because of a typo

	// using size of amplitude vectors for all other detectors of the same type

	// cross detectors
	for(size_t i = 0; i < crossAmplitude.size(); ++i) {
		try {
			fH1[slot].at("crossE")->Fill(fCalibration->Energy(crossAmplitude[i], i));
		} catch(std::exception& e) {
			std::cout << "Failed to fill histogram for cross channel " << i << " (amplitude " << crossAmplitude[i] << "): " << e.what() << std::endl;
		}
		if(i > 0) {
			fH2[slot].at("crossT")->Fill(fCalibration->Time(crossAmplitude[i]) - fCalibration->Time(crossAmplitude[0]), i);
		}
	}

	// back detectors
	for(size_t i = 0; i < backAmplitude.size(); ++i) {
		fH1[slot].at("backE")->Fill(fCalibration->Energy(backAmplitude[i], i + 16));
	}

	// misc detectors
	for(size_t i = 0; i < miscAmplitude.size(); ++i) {
		fH1[slot].at("miscE")->Fill(fCalibration->Energy(miscAmplitude[i], i + 32));
	}

	// cebr detectors
	for(size_t i = 0; i < cebrIntLong.size(); ++i) {
		fH1[slot].at("cebrCh")->Fill(cebrIntLong[i]);
	}
	
	// addbackl
	double addback = 0.;
	for(size_t i = 0; i < crossAmplitude.size(); ++i) {
		if(!std::isnan(crossAmplitude[i])) {
			addback += fCalibration->Energy(crossAmplitude[i], i);
		}
		// check if this index is the last crystal of a detector
		// assuming 0-3 are the crystals of the first detector, 4-7 the second detector and so on?
		if(i%4 == 3 && addback > 0.) {
			fH1[slot].at("crossAddbackE")->Fill(addback);
			addback = 0.;
		}
	}

	// hit pattern (with check that amplitudes are not NaN)
   // meed all combinations of detector type	
	for(size_t i = 0; i < crossAmplitude.size(); ++i) {
		if(std::isnan(crossAmplitude[i])) { continue; }
		for(size_t j = 0; j < crossAmplitude.size(); ++j) {
			if(i == j || std::isnan(crossAmplitude[j])) { continue; }
			fH2[slot].at("hp")->Fill(i, j);
		}
		for(size_t j = 0; j < backAmplitude.size(); ++j) {
			if(std::isnan(backAmplitude[j])) { continue; }
			fH2[slot].at("hp")->Fill(i, j + 16);
			fH2[slot].at("hp")->Fill(j + 16, i);
		}
		for(size_t j = 0; j < miscAmplitude.size(); ++j) {
			if(std::isnan(miscAmplitude[j])) { continue; }
			fH2[slot].at("hp")->Fill(i, j + 32);
			fH2[slot].at("hp")->Fill(j + 32, i);
		}
		for(size_t j = 0; j < cebrIntLong.size(); ++j) {
			if(std::isnan(cebrIntLong[j])) { continue; }
			fH2[slot].at("hp")->Fill(i, j + 48);
			fH2[slot].at("hp")->Fill(j + 48, i);
		}
	}
	for(size_t i = 0; i < backAmplitude.size(); ++i) {
		if(std::isnan(backAmplitude[i])) { continue; }
		for(size_t j = 0; j < backAmplitude.size(); ++j) {
			if(i == j || std::isnan(backAmplitude[j])) { continue; }
			fH2[slot].at("hp")->Fill(i + 16, j + 16);
		}
		for(size_t j = 0; j < miscAmplitude.size(); ++j) {
			if(std::isnan(miscAmplitude[j])) { continue; }
			fH2[slot].at("hp")->Fill(i + 16, j + 32);
			fH2[slot].at("hp")->Fill(j + 32, i + 16);
		}
		for(size_t j = 0; j < cebrIntLong.size(); ++j) {
			if(std::isnan(cebrIntLong[j])) { continue; }
			fH2[slot].at("hp")->Fill(i + 16, j + 48);
			fH2[slot].at("hp")->Fill(j + 48, i + 16);
		}
	}
	for(size_t i = 0; i < miscAmplitude.size(); ++i) {
		if(std::isnan(miscAmplitude[i])) { continue; }
		for(size_t j = 0; j < miscAmplitude.size(); ++j) {
			if(i == j || std::isnan(miscAmplitude[j])) { continue; }
			fH2[slot].at("hp")->Fill(i + 32, j + 32);
		}
		for(size_t j = 0; j < cebrIntLong.size(); ++j) {
			if(std::isnan(cebrIntLong[j])) { continue; }
			fH2[slot].at("hp")->Fill(i + 32, j + 48);
			fH2[slot].at("hp")->Fill(j + 48, i + 32);
		}
	}
	for(size_t i = 0; i < cebrIntLong.size(); ++i) {
		if(std::isnan(cebrIntLong[i])) { continue; }
		for(size_t j = 0; j < cebrIntLong.size(); ++j) {
			if(i == j || std::isnan(cebrIntLong[j])) { continue; }
			fH2[slot].at("hp")->Fill(i + 48, j + 48);
		}
	}
}

void ExampleHelper::EndOfSort(std::shared_ptr<std::map<std::string, TList>>& list)
{
}
