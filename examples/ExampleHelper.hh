// TODO: Replace all EXAMPLEVENT and Example with the name you want to use for this helper action
#ifndef EXAMPLEHELPER_HH
#define EXAMPLEHELPER_HH

#include "BasicHelper.h"

// This is a custom action which respects a well defined interface. It supports parallelism,
// in the sense that it behaves correctly if implicit multi threading is enabled.
// Note the plural: in presence of a MT execution, internally more than a single TList is created.
// The detector types in the specifcation of Book must match those in the call to it as well as those in the Exec function (and be in the same order)!

class ExampleHelper : public BasicHelper, public ROOT::Detail::RDF::RActionImpl<ExampleHelper> {
public:
   // constructor sets the prefix (which is used for the output file as well)
   // and calls Setup which in turn also calls CreateHistograms
   explicit ExampleHelper(TList* list)
      : BasicHelper(list)
   {
      Prefix("ExampleHelper");
      Setup();
   }

   ROOT::RDF::RResultPtr<std::map<std::string, TList>> Book(ROOT::RDataFrame* d) override
   {
      // TODO: edit the template specification and branch names to match the detectors you want to use!
      return d->Book<ROOT::RVecD, ROOT::RVecD, ROOT::RVecD, ROOT::RVecD, ROOT::RVecD, ROOT::RVecD, ROOT::RVecD, ROOT::RVecD, ROOT::RVecD, ROOT::RVecD, ROOT::RVecD, ROOT::RVecD, ROOT::RVecD, ROOT::RVecD, ROOT::RVecD, ROOT::RVecD, ROOT::RVecD, ROOT::RVecD, ROOT::RVecD, ROOT::RVecD>(std::move(*this), {"clover_cross.amplitude", "clover_cross.channel_time", "clover_cross.module_timestamp", "clover_cross.pileup", "clover_cross.trigger_time", "extended_timestamp", "clover_back.amplitude", "clover_back.channel_time", "clover_back.module_timestamp", "clover_back.pileup", "clover_back.trigger_time", "misc.amplitude", "misc.channel_time", "misc.module_timestamp", "misc.pileup", "misc.trigger_time", "cebr_all.channel_time", "cebr_all.integration_long", "cebr_all.module_timestamp", "cebr_all.trigger_time"});
   }
   // this function creates and books all histograms
   void CreateHistograms(unsigned int slot) override;
   // this function gets called for every single event and fills the histograms
   // TODO: edit the function arguments to match the detectors you want to use!
   void Exec(unsigned int slot, ROOT::RVecD& crossAmplitude, ROOT::RVecD& crossChannelTime, ROOT::RVecD& crossModuleTS, ROOT::RVecD& crossPileup, ROOT::RVecD& crossTriggerTime, ROOT::RVecD& extendedTS, ROOT::RVecD& backAmplitude, ROOT::RVecD& backChannelTime, ROOT::RVecD& backModuleTS, ROOT::RVecD& backPileup, ROOT::RVecD& backTriggerTime, ROOT::RVecD& miscAmplitude, ROOT::RVecD& miscChannelTime, ROOT::RVecD& miscModuleTS, ROOT::RVecD& miscPileup, ROOT::RVecD& miscTriggerTime, ROOT::RVecD& cebrChannelTime, ROOT::RVecD& cebrIntLong, ROOT::RVecD& cebrModuleTS, ROOT::RVecD& cebrTriggerTime);
   // this function is optional and is called after the output lists off all slots/workers have been merged
   void EndOfSort(std::shared_ptr<std::map<std::string, TList>>& list) override;

private:
   // any constants that are set in the CreateHistograms function and used in the Exec function can be stored here
   // or any other settings
};

// These are needed functions used by TDataFrameLibrary to create and destroy the instance of this helper
extern "C" ExampleHelper* CreateHelper(TList* list) { return new ExampleHelper(list); }

extern "C" void DestroyHelper(BasicHelper* helper) { delete helper; }

#endif
