//------------------------------------------------------------------------
// Copyright(c) 2022 LumyLumyStudio.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "myplugincids.h"
#include "AudioBuffer.h"
#include "SharedM.h"

namespace LumyLumyStudio {


//------------------------------------------------------------------------
//  RoutePePeProcessor
//------------------------------------------------------------------------
class RoutePePeProcessor : public Steinberg::Vst::AudioEffect
{
public:
  RoutePePeProcessor ();
  ~RoutePePeProcessor () SMTG_OVERRIDE;

  // Create function
  static Steinberg::FUnknown* createInstance (void* /*context*/) 
  { 
    return (Steinberg::Vst::IAudioProcessor*)new RoutePePeProcessor; 
  }

  //--- ---------------------------------------------------------------------
  // AudioEffect overrides:
  //--- ---------------------------------------------------------------------
  /** Called at first after constructor */
  Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
  
  /** Called at the end before destructor */
  Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;
  
  /** Switch the Plug-in on/off */
  Steinberg::tresult PLUGIN_API setActive (Steinberg::TBool state) SMTG_OVERRIDE;

  /** Will be called before any process call */
  Steinberg::tresult PLUGIN_API setupProcessing (Steinberg::Vst::ProcessSetup& newSetup) SMTG_OVERRIDE;
  
  /** Asks if a given sample size is supported see SymbolicSampleSizes. */
  Steinberg::tresult PLUGIN_API canProcessSampleSize (Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;

  /** Here we go...the process call */
  Steinberg::tresult PLUGIN_API process (Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;
    
  /** For persistence */
  Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
  Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;

//------------------------------------------------------------------------
protected:



  //-------------------------
  //field of RoutePePeProcessor - test
  //-------------------------

  //
  ;

  //-------------------------
  //field of RoutePePeProcessor - options, state
  //-------------------------

  //
  bool doUpdate = true;
  bool f_noCvt = false;
  //
  int sharedMem_intname = 0;
  int iPort_count = 0;
  int oPort_count = 0;
  //
  size_t bufSize = (size_t)0;
  size_t bufsync_prefill = (size_t)0;
  size_t bufsync_low = (size_t)0;
  size_t bufsync_high = (size_t)0;
  size_t prefill_synccounts = (size_t)0;
  //
  size_t bufavgpos_counts = (size_t)0;
  size_t noresample_len = (size_t)0;

  //
  bool doUpdate_v = false;
  bool f_noCvt_v = false;
  //
  int sharedMem_intname_v = 0;
  int iPort_count_v = 2;
  int oPort_count_v = 2;
  //
  size_t bufSize_v = (size_t)4096;
  size_t bufsync_prefill_v = (size_t)2048;
  size_t bufsync_low_v = (size_t)0;
  size_t bufsync_high_v = (size_t)4096;
  size_t prefill_synccounts_v = (size_t)16;
  //
  size_t bufavgpos_counts_v = (size_t)4096;
  size_t noresample_len_v = (size_t)64;

  //-------------------------
  //field of RoutePePeProcessor - buffer
  //-------------------------

  //
  SharedM* iPort_SharedM[MAX_PORTS] = { NULL, };
  AudioBufferOnMEM* iPort_Sbuffers[MAX_PORTS] = { NULL, };
  SharedM* oPort_SharedM[MAX_PORTS] = { NULL, };
  AudioBufferOnMEM* oPort_Sbuffers[MAX_PORTS] = { NULL, };
  //
  size_t oPort_bufavgpos_first[MAX_PORTS] = { (size_t)0, };
  size_t oPort_bufavgpos[MAX_PORTS] = { (size_t)0,};
  long long int oPort_bufavgpos_sum[MAX_PORTS] = { (long long int)0,};
  size_t oPort_bufavgpos_count[MAX_PORTS] = { (size_t)0, };
  bool prefillSyncState[MAX_PORTS] = { false, };

  //-------------------------
  //field of RoutePePeProcessor - statistic
  //-------------------------

  //
  bool iConnectionStarted = false;
  bool oConnectionStarted = false;
  size_t c_resync_low = (size_t)0;
  size_t c_resync_high = (size_t)0;
  size_t c_resample_low = (size_t)0;
  size_t c_resample_high = (size_t)0;
  size_t c_overflow = (size_t)0;
  size_t c_32missing = (size_t)0;

};

//------------------------------------------------------------------------
} // namespace LumyLumyStudio
