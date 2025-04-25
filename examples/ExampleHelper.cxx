#include "ExampleHelper.hh"

void ExampleHelper::CreateHistograms(unsigned int slot)
{
   // some variables to easily change range and binning for multiple histograms at once
   int    energyBins = 10000;
   double lowEnergy  = 0.;
   double highEnergy = 2000.;

   fH1[slot]["crossE"] = new TH1F("crossE", Form("Cross energy;energy [keV];counts/%.1f keV", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);

   fH2[slot]["crossT"] = new TH2F("crossT", "Cross ID vs timinig relative to Cross_{0};time [ns];Cross ID", 1000, -5000., 5000., 15, 0.5, 15.5);
}

// TODO: Change the function arguments to match the detectors you want to use and the declaration in the header file!
void ExampleHelper::Exec(unsigned int slot, ROOT::RVecD& crossAmplitude, ROOT::RVecD& crossChannelTime, ROOT::RVecD& crossModuleTS, ROOT::RVecD& crossPileup, ROOT::RVecD& crossTriggerTime, ROOT::RVecD& extendedTS)
{
   // we use .at() here instead of [] so that we get meaningful error message if a histogram we try to fill wasn't created
   // e.g. because of a typo

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
}

void ExampleHelper::EndOfSort(std::shared_ptr<std::map<std::string, TList>>& list)
{
}
