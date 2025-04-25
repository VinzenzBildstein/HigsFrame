#include "Calibration.h"

#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

Calibration::Calibration(const char* file)
{
	std::ifstream input(file);

	if(!input.is_open()) {
		std::cout << "Failed to open calibration file \"" << file << "\"!" << std::endl;
	} else {
		std::string line;
		int valuesRead = 0;
		while(std::getline(input, line).good()) {
			if(line.empty() || line[0] == '#') {
				continue;
			}
			std::stringstream str(line);
			if(valuesRead < 48) {
				double tmp;
				str >> tmp;
				fOffset.push_back(tmp);
				str >> tmp;
				fGain.push_back(tmp);
			} else if(valuesRead == 48) {
				str >> fTimeOffset >> fTimeGain;
			} else if(valuesRead == 49) {
				str >> fTimestampOffset >> fTimestampGain;
			}
			++valuesRead;
		}
	}
}

void Calibration::Print(Option_t*) const
{
	std::cout << "Got " << fGain.size() << " energy calibrations" << std::endl;

	std::cout << "Type          Channel  Offset    Gain      keV(0)    keV(65536)" << std::endl;

	for(int i = 0; i < 16; ++i) {
		if(i < 16) {
			std::cout << "clover_cross  ";
		} else if(i < 32) {
			std::cout << "clover_back   ";
		} else {
			std::cout << "misc          ";
		}
		std::cout << std::setw(7) << i << "  " << std::setw(8) << fOffset[i] << "  " << std::setw(8) << fGain[i] << "  " << std::setw(8) << fOffset[i] << "  " << std::setw(8) << fOffset[i] + fGain[i] * 65536. << std::endl;
	}

	std::cout << std::endl;

	std::cout << "Time calibration: offset " << fTimeOffset << ", gain " << fTimeGain << " => time at channel 65536: " << fTimeOffset + fTimeGain * 65536. << " ns" << std::endl;
	std::cout << "Timestamp calibration: offset " << fTimestampOffset << ", gain " << fTimestampGain << " => time at channel 65536: " << fTimestampOffset + fTimestampGain * 65536. << " ns" << std::endl;
}
