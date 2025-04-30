#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "TObject.h"
#include "TRandom.h"

class Calibration : public TObject {
public:
   Calibration() = default;
   explicit Calibration(const char* file);

   double Energy(double channel, int id) const { return fOffset.at(id) + fGain.at(id) * (channel + gRandom->Uniform(0, 1)); }

   double Time(double channel) const { return fTimeOffset + fTimeGain * (channel + gRandom->Uniform(0, 1)); }

   double Timestamp(double channel) const { return fTimestampOffset + fTimestampGain * (channel + gRandom->Uniform(0, 1)); }

   void Print(Option_t* opt = "") const override;

private:
   std::vector<double> fOffset;
   std::vector<double> fGain;
   double              fTimeOffset{0.};
   double              fTimeGain{1.};
   double              fTimestampOffset{0.};
   double              fTimestampGain{1.};

   ClassDefOverride(Calibration, 1);   // NOLINT(readability-else-after-return)
};

#endif
