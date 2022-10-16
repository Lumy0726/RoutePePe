//------------------------------------------------------------------------
// Copyright(c) 2022 LumyLumyStudio.
//------------------------------------------------------------------------

#include "mypluginprocessor.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/vst/vstaudioprocessoralgo.h"

#include <math.h>

using namespace Steinberg;

namespace LumyLumyStudio {
//------------------------------------------------------------------------
// RoutePePeProcessor
//------------------------------------------------------------------------
RoutePePeProcessor::RoutePePeProcessor ()
{
  //--- set the wanted controller for our processor
  setControllerClass (kRoutePePeControllerUID);
}

//------------------------------------------------------------------------
RoutePePeProcessor::~RoutePePeProcessor ()
{
  ;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RoutePePeProcessor::initialize (FUnknown* context)
{
  // Here the Plug-in will be instanciated
  
  //---always initialize the parent-------
  tresult result = AudioEffect::initialize (context);
  // if everything Ok, continue
  if (result != kResultOk)
  {
    return result;
  }

  //--- create Audio IO ------
  addAudioInput (STR16 ("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
  addAudioOutput (STR16 ("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

  /* If you don't need an event bus, you can remove the next line */
  //addEventInput (STR16 ("Event In"), 1);

  return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RoutePePeProcessor::terminate ()
{
  // Here the Plug-in will be de-instanciated, last possibility to remove some memory!

  //*
  for (size_t i = (size_t)0; i < MAX_PORTS; i++) {
    delete iPort_SharedM[i];
    delete oPort_SharedM[i];
    delete iPort_Sbuffers[i];
    delete oPort_Sbuffers[i];
  }
  //*/
  
  //---do not forget to call parent ------
  return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API RoutePePeProcessor::setActive (TBool state)
{
  //--- called when the Plug-in is enable/disable (On/Off) -----

  bool noerrorB = true;
  //memory_mode. 0 is init value, 1 is creation mode, 2 is opening mode.
  int memory_mode = 0;
  char sharedMem_name[64] = { (char)0, };
  char sharedMem_name_c[64] = { (char)0, };
  char sharedMem_name_o[64] = { (char)0, };

  if (state && (doUpdate || doUpdate_v)) {
    //need to apply some settings and (re)allocate shared memory.


    //delete shared memory.
    for (size_t i = (size_t)0; i < MAX_PORTS; i++) {
      delete iPort_SharedM[i];
      iPort_SharedM[i] = NULL;
      delete oPort_SharedM[i];
      oPort_SharedM[i] = NULL;
      delete iPort_Sbuffers[i];
      iPort_Sbuffers[i] = NULL;
      delete oPort_Sbuffers[i];
      oPort_Sbuffers[i] = NULL;
    }


    //apply some settings value.
    doUpdate = false;
    f_noCvt = f_noCvt_v;
    sharedMem_intname = sharedMem_intname_v;
    iPort_count = iPort_count_v;
    oPort_count = oPort_count_v;
    bufSize = bufSize_v + (size_t)1;//add one more sample, for (bufsync_high) effect.
    if (bufSize < (size_t)2) bufSize = (size_t)2;
    else if (bufSize > (size_t)BUFFER_ETC_MAX_SIZE) bufSize = (size_t)BUFFER_ETC_MAX_SIZE;
    size_t memSize = AudioBufferOnMEM::bufLen2memSize(bufSize);
    //init statistic
    iConnectionStarted = false;
    oConnectionStarted = false;
    c_resync_low = (size_t)0;
    c_resync_high = (size_t)0;
    c_resample_low = (size_t)0;
    c_resample_high = (size_t)0;
    c_overflow = (size_t)0;
    c_32missing = (size_t)0;


    //make (or open) shared memory.
    if (sharedMem_intname == 0) {
      //CASE OF shared memory id (int name) is 0 (default value).
      iPort_count = oPort_count = 0;
    }
    else {
      sprintf_s(sharedMem_name, "RoutePePe_%05d", sharedMem_intname);
      //input ports
      for (int i = 0; i < iPort_count; i++) {
        try {
          iPort_SharedM[i] = new SharedM;
          iPort_Sbuffers[i] = new AudioBufferOnMEM;
        }
        catch (std::exception e) {
          noerrorB = false;
          break;
        }
        sprintf_s(sharedMem_name_c, "%s_input%03d", sharedMem_name, i);
        sprintf_s(sharedMem_name_o, "%s_output%03d", sharedMem_name, i);
        switch (memory_mode) {
        case 0:
          noerrorB = iPort_SharedM[i]->open_mem(sharedMem_name_o);
          if (noerrorB) {
            memory_mode = 2;
          }
          else {
            memory_mode = 1;
            noerrorB = iPort_SharedM[i]->init_mem(sharedMem_name_c, memSize);
          }
          break;
        case 1:
          noerrorB = iPort_SharedM[i]->init_mem(sharedMem_name_c, memSize);
          break;
        case 2:
          noerrorB = iPort_SharedM[i]->open_mem(sharedMem_name_o);
          break;
        }
        if (!noerrorB) { break; }
        switch (memory_mode) {
        case 1:
          noerrorB = iPort_Sbuffers[i]->init_class(iPort_SharedM[i]->get_mem_contAddr(), memSize);
          break;
        case 2:
          iPort_Sbuffers[i]->load_class(iPort_SharedM[i]->get_mem_contAddr(), iPort_SharedM[i]->get_mem_contSize());
          break;
        }
        if (!noerrorB) { break; }
      }
      if (!noerrorB) { oPort_count = 0; }
      //output ports
      for (int i = 0; i < oPort_count; i++) {
        try {
          oPort_SharedM[i] = new SharedM;
          oPort_Sbuffers[i] = new AudioBufferOnMEM;
        }
        catch (std::exception e) {
          noerrorB = false;
          break;
        }
        sprintf_s(sharedMem_name_c, "%s_output%03d", sharedMem_name, i);
        sprintf_s(sharedMem_name_o, "%s_input%03d", sharedMem_name, i);
        switch (memory_mode) {
        case 0:
          noerrorB = oPort_SharedM[i]->open_mem(sharedMem_name_o);
          if (noerrorB) {
            memory_mode = 2;
          }
          else {
            memory_mode = 1;
            noerrorB = oPort_SharedM[i]->init_mem(sharedMem_name_c, memSize);
          }
          break;
        case 1:
          noerrorB = oPort_SharedM[i]->init_mem(sharedMem_name_c, memSize);
          break;
        case 2:
          noerrorB = oPort_SharedM[i]->open_mem(sharedMem_name_o);
          break;
        }
        if (!noerrorB) { break; }
        switch (memory_mode) {
        case 1:
          noerrorB = oPort_Sbuffers[i]->init_class(oPort_SharedM[i]->get_mem_contAddr(), memSize);
          break;
        case 2:
          oPort_Sbuffers[i]->load_class(oPort_SharedM[i]->get_mem_contAddr(), oPort_SharedM[i]->get_mem_contSize());
          break;
        }
        if (!noerrorB) { break; }
      }
      //for memory open mode, confirm memory init, get real "bufSize", (but may this may be corrupted value).
      if (noerrorB && memory_mode == 2) {
        if (iPort_count > 0) {
          if (!(iPort_SharedM[0]->mem_catchUnlock((size_t)500, 1000))) {
            noerrorB = false;
          }
          else {
            iPort_SharedM[0]->mem_lock(SMEMORY_LOCK_INTERVAL);
            bufSize = iPort_Sbuffers[0]->get_buf_contSize();
            iPort_SharedM[0]->mem_unlock();
          }
        }
        if (oPort_count > 0) {
          if (!(oPort_SharedM[0]->mem_catchUnlock((size_t)500, 1000))) {
            noerrorB = false;
          }
          else {
            oPort_SharedM[0]->mem_lock(SMEMORY_LOCK_INTERVAL);
            bufSize = oPort_Sbuffers[0]->get_buf_contSize();
            oPort_SharedM[0]->mem_unlock();
          }
        }
        if (bufSize < (size_t)2) bufSize = (size_t)2;
        else if (bufSize > (size_t)BUFFER_ETC_MAX_SIZE) bufSize = (size_t)BUFFER_ETC_MAX_SIZE;
      }
      //fill iPort buffer.
      if (noerrorB) {
        if (iPort_count > 0) {
          iPort_SharedM[0]->mem_lock(SMEMORY_LOCK_INTERVAL);
          for (int i = 0; i < iPort_count; i++) {
            iPort_Sbuffers[i]->push_fill((AB_stype)0, (size_t)0);
          }
          iPort_SharedM[0]->mem_unlock();
        }
      }
    }
    if (!noerrorB) {
      //CASE OF memory creation/opening error.
      iPort_count = oPort_count = 0;
      //delete shared memory.
      for (size_t i = (size_t)0; i < MAX_PORTS; i++) {
        delete iPort_SharedM[i];
        iPort_SharedM[i] = NULL;
        delete oPort_SharedM[i];
        oPort_SharedM[i] = NULL;
        delete iPort_Sbuffers[i];
        iPort_Sbuffers[i] = NULL;
        delete oPort_Sbuffers[i];
        oPort_Sbuffers[i] = NULL;
      }
    }


    //apply some settings value.
    bufsync_prefill = bufsync_prefill_v;
    bufsync_low = bufsync_low_v;
    bufsync_high = bufsync_high_v;
    if (bufSize <= bufsync_prefill && bufSize > (size_t)0) bufsync_prefill = bufSize - (size_t)1;
    if (bufSize <= bufsync_high && bufSize > (size_t)0) bufsync_high = bufSize - (size_t)1;
    if (bufsync_high < bufsync_prefill) bufsync_high = bufsync_prefill;
    if (bufsync_prefill < bufsync_low) bufsync_low = bufsync_prefill;
    prefill_synccounts = prefill_synccounts_v;
    bufavgpos_counts = bufavgpos_counts_v;
    noresample_len = noresample_len_v;
    //init value related with "bufavgpos", init value "prefillSyncState"
    for (int i = 0; i < MAX_PORTS; i++) {
      oPort_bufavgpos_first[i] = (size_t)0;
      oPort_bufavgpos[i] = (size_t)0;
      oPort_bufavgpos_sum[i] = (long long int)0;
      oPort_bufavgpos_count[i] = (size_t)0;
      prefillSyncState[i] = false;
    }
    //init statistic
    if (iPort_count == 0 && oPort_count > 0) {
      iConnectionStarted = true;
    }
    else if (iPort_count > 0 && oPort_count == 0) {
      oConnectionStarted = true;
    }
  }

  return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API RoutePePeProcessor::process (Vst::ProcessData& data)
{
  //Read inputs parameter changes

  //
  int intTemp, intTemp2;
  if (data.inputParameterChanges) {
    int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
    for (int32 index = 0; index < numParamsChanged; index++) {
      Vst::IParamValueQueue* pvq = data.inputParameterChanges->getParameterData(index);
      if (pvq) {
        Vst::ParamValue value;
        int32 sampleOffset;
        int32 numPoints = pvq->getPointCount();
        tresult ret;
        ret = pvq->getPoint(numPoints - (int32)1, sampleOffset, value);
        if (ret == kResultOk) {

          //input param
          switch (pvq->getParameterId()) {

            //
          case RPPID::DO_UPDATE:
            if (value > (Vst::ParamValue)0.5) {
              doUpdate_v = true;
            }
            else {
              doUpdate_v = false;
            }
            break;
            //
          case RPPID::F_NOCVT:
            if (value > (Vst::ParamValue)0.5) {
              f_noCvt_v = true;
            }
            else {
              f_noCvt_v = false;
            }
            break;

            //
          case RPPID::SMEM_ID:
            intTemp = (int)(::round((double)(value) * SMEMORY_INTNAME_MAX));
            sharedMem_intname_v = intTemp;
            if (sharedMem_intname_v < 0) sharedMem_intname_v = 0;
            else if (sharedMem_intname_v > SMEMORY_INTNAME_MAX) sharedMem_intname_v = SMEMORY_INTNAME_MAX;
            break;
            //
          case RPPID::ICOUNT:
            intTemp = (int)(::round((double)(value) * MAX_PORTS));
            iPort_count_v = intTemp;
            if (iPort_count_v < 0) iPort_count_v = 0;
            else if (iPort_count_v > MAX_PORTS) iPort_count_v = MAX_PORTS;
            break;
            //
          case RPPID::OCOUNT:
            intTemp = (int)(::round((double)(value) * MAX_PORTS));
            oPort_count_v = intTemp;
            if (oPort_count_v < 0) oPort_count_v = 0;
            else if (oPort_count_v > MAX_PORTS) oPort_count_v = MAX_PORTS;
            break;

            //
          case RPPID::BUFSIZE:
            intTemp = (int)(::round((double)(value) * (double)BUFFER_OPTION_COUNT));
            bufSize_v = (size_t)int2regularSize(intTemp, BUFFER_HALFSCALE);
            if (bufSize_v > (size_t)BUFFER_ETC_MAX_SIZE) bufSize_v = (size_t)BUFFER_ETC_MAX_SIZE;
            break;
            //
          case RPPID::BUFSYNC_PREFILL:
            intTemp = (int)(::round((double)(value) * (double)BUFFER_OPTION_COUNT));
            bufsync_prefill_v = (size_t)int2regularSize(intTemp, BUFFER_HALFSCALE);
            if (bufsync_prefill_v > (size_t)BUFFER_ETC_MAX_SIZE) bufsync_prefill_v = (size_t)BUFFER_ETC_MAX_SIZE;
            if (oPort_count > 0) {
              oPort_SharedM[0]->mem_lock(SMEMORY_LOCK_INTERVAL);
              for (int i = 0; i < oPort_count; i++) {
                oPort_Sbuffers[i]->push_fill((AB_stype)0, (size_t)0);
              }
              oPort_SharedM[0]->mem_unlock();
            }
            break;
            //
          case RPPID::BUFSYNC_LOW:
            intTemp = (int)(::round((double)(value) * (double)BUFFER_OPTION_COUNT));
            bufsync_low_v = (size_t)int2regularSize(intTemp, BUFFER_HALFSCALE);
            if (bufsync_low_v > (size_t)BUFFER_ETC_MAX_SIZE) bufsync_low_v = (size_t)BUFFER_ETC_MAX_SIZE;
            break;
            //
          case RPPID::BUFSYNC_HIGH:
            intTemp = (int)(::round((double)(value) * (double)BUFFER_OPTION_COUNT));
            bufsync_high_v = (size_t)int2regularSize(intTemp, BUFFER_HALFSCALE);
            if (bufsync_high_v > (size_t)BUFFER_ETC_MAX_SIZE) bufsync_high_v = (size_t)BUFFER_ETC_MAX_SIZE;
            break;
            //
          case RPPID::PREFILL_SYNC_COUNTS:
            intTemp = (int)(::round((double)(value) * (double)BUFFER_OPTION_COUNT));
            prefill_synccounts_v = (size_t)int2regularSize(intTemp, BUFFER_HALFSCALE);
            if (prefill_synccounts_v > (size_t)BUFFER_ETC_MAX_SIZE) prefill_synccounts_v = (size_t)BUFFER_ETC_MAX_SIZE;
            break;

            //
          case RPPID::BUFAVGPOS_COUNTS:
            intTemp = (int)(::round((double)(value) * (double)BUFFER_OPTION_COUNT));
            bufavgpos_counts_v = (size_t)int2regularSize(intTemp, BUFFER_HALFSCALE);
            if (bufavgpos_counts_v > (size_t)BUFFER_ETC_MAX_SIZE) bufavgpos_counts_v = (size_t)BUFFER_ETC_MAX_SIZE;
            break;
            //
          case RPPID::NORESAMPLE_LEN:
            intTemp = (int)(::round((double)(value) * (double)BUFFER_OPTION_COUNT));
            noresample_len_v = (size_t)int2regularSize(intTemp, BUFFER_HALFSCALE);
            if (noresample_len_v > (size_t)BUFFER_ETC_MAX_SIZE) noresample_len_v = (size_t)BUFFER_ETC_MAX_SIZE;
            break;

            //
          case RPPID::S_RESYNC:
            intTemp = (int)(::round((double)(value) * (
              (STATISTIC_MAXCOUNT + 1) * (STATISTIC_MAXCOUNT + 1) - 1
              )));
            intTemp2 = intTemp % (STATISTIC_MAXCOUNT + 1);
            intTemp = intTemp / (STATISTIC_MAXCOUNT + 1);
            c_resync_low = (size_t)intTemp2;
            if (c_resync_low < (size_t)0) c_resync_low = (size_t)0;
            else if (c_resync_low > (size_t)STATISTIC_MAXCOUNT) c_resync_low = (size_t)STATISTIC_MAXCOUNT;
            c_resync_high = (size_t)intTemp;
            if (c_resync_high < (size_t)0) c_resync_high = (size_t)0;
            else if (c_resync_high > (size_t)STATISTIC_MAXCOUNT) c_resync_high = (size_t)STATISTIC_MAXCOUNT;
            break;
            //
          case RPPID::S_RESAMPLE:
            intTemp = (int)(::round((double)(value) * (
              (STATISTIC_MAXCOUNT + 1) * (STATISTIC_MAXCOUNT + 1) - 1
              )));
            intTemp2 = intTemp % (STATISTIC_MAXCOUNT + 1);
            intTemp = intTemp / (STATISTIC_MAXCOUNT + 1);
            c_resample_low = (size_t)intTemp2;
            if (c_resample_low < (size_t)0) c_resample_low = (size_t)0;
            else if (c_resample_low > (size_t)STATISTIC_MAXCOUNT) c_resample_low = (size_t)STATISTIC_MAXCOUNT;
            c_resample_high = (size_t)intTemp;
            if (c_resample_high < (size_t)0) c_resample_high = (size_t)0;
            else if (c_resample_high > (size_t)STATISTIC_MAXCOUNT) c_resample_high = (size_t)STATISTIC_MAXCOUNT;
            break;
            //
          case RPPID::S_OVERFLOW:
            intTemp = (int)(::round((double)(value)*STATISTIC_MAXCOUNT));
            c_overflow = (size_t)intTemp;
            if (c_overflow < (size_t)0) c_overflow = (size_t)0;
            else if (c_overflow > (size_t)STATISTIC_MAXCOUNT) c_overflow = (size_t)STATISTIC_MAXCOUNT;
            break;
            //
          case RPPID::S_32MISSING:
            intTemp = (int)(::round((double)(value)*STATISTIC_MAXCOUNT));
            c_32missing = (size_t)intTemp;
            if (c_32missing < (size_t)0) c_32missing = (size_t)0;
            else if (c_32missing > (size_t)STATISTIC_MAXCOUNT) c_32missing = (size_t)STATISTIC_MAXCOUNT;
            break;
          }


        }
      }
    }

    //apply some settings value.
    bufsync_prefill = bufsync_prefill_v;
    bufsync_low = bufsync_low_v;
    bufsync_high = bufsync_high_v;
    if (bufSize <= bufsync_prefill && bufSize > (size_t)0) bufsync_prefill = bufSize - (size_t)1;
    if (bufSize <= bufsync_high && bufSize > (size_t)0) bufsync_high = bufSize - (size_t)1;
    if (bufsync_high < bufsync_prefill) bufsync_high = bufsync_prefill;
    if (bufsync_prefill < bufsync_low) bufsync_low = bufsync_prefill;
    prefill_synccounts = prefill_synccounts_v;
    bufavgpos_counts = bufavgpos_counts_v;
    noresample_len = noresample_len_v;
  }


  //audio processing

  //
  size_t bytes_len = (size_t)Vst::getSampleFramesSizeInBytes(processSetup, data.numSamples);
  void** in = Vst::getChannelBuffersPointer(processSetup, data.inputs[0]);
  void** out = Vst::getChannelBuffersPointer(processSetup, data.outputs[0]);
  int iCcount = (int)(data.inputs[0].numChannels);
  int oCcount = (int)(data.outputs[0].numChannels);
  data.outputs[0].silenceFlags = (Steinberg::uint64)0;
  if (iPort_count < iCcount) iCcount = iPort_count;
  if (oPort_count < oCcount) oCcount = oPort_count;
  //
  size_t curLen;
  size_t emptyLen;
  size_t moveLen = (size_t)0;
  //
  bool needParamUpdate = false;
  bool _iConnectionStarted = false;
  size_t resyncL = (size_t)0;
  size_t resyncH = (size_t)0;
  size_t resampleL = (size_t)0;
  size_t resampleH = (size_t)0;
  size_t overflow = (size_t)0;
  size_t _32missing = (size_t)0;

  //
  if (processSetup.symbolicSampleSize == Vst::kSample64 || f_noCvt) {
    if (bufavgpos_counts > (size_t)0) {
      //CASE OF 64bit in/out, or 32bit with no conversion.
      //CASE OF resample enabled.



      moveLen = bytes_len / sizeof(AB_stype);
      if (moveLen * sizeof(AB_stype) != bytes_len) {
        _32missing = (size_t)1;
        needParamUpdate = true;
      }

      //For input
      if (iPort_count > 0 && moveLen > (size_t)0) {
        iPort_SharedM[0]->mem_lock(SMEMORY_LOCK_INTERVAL);
        for (int i = 0; i < iCcount; i++) {
          emptyLen = iPort_Sbuffers[i]->get_curEmptyLen();
          if (moveLen > emptyLen) {
            iPort_Sbuffers[i]->pop_count(moveLen - emptyLen);
            overflow = (size_t)1;
            needParamUpdate = true;
          }
          else if (!iConnectionStarted) {
            _iConnectionStarted = true;
            needParamUpdate = true;
          }
          iPort_Sbuffers[i]->push_arr((AB_stype*)(in[i]), moveLen);
        }
        iPort_SharedM[0]->mem_unlock();
      }
      //For output
      if (oPort_count > 0 && moveLen > (size_t)0) {
        oPort_SharedM[0]->mem_lock(SMEMORY_LOCK_INTERVAL);
        for (int i = 0; i < oCcount; i++) {
          curLen = oPort_Sbuffers[i]->get_curLen();
          if (prefillSyncState[i]) {
            oPort_bufavgpos_sum[i] += (long long int)curLen;
            if (++(oPort_bufavgpos_count[i]) >= prefill_synccounts) {
              curLen = (size_t)(oPort_bufavgpos_sum[i] / (long long int)(oPort_bufavgpos_count[i]));
              oPort_bufavgpos_sum[i] = (double)0.0;
              oPort_bufavgpos_count[i] = (size_t)0;
              if (curLen > bufsync_prefill) {
                oPort_Sbuffers[i]->pop_count(curLen - bufsync_prefill);
              }
              else if (curLen < bufsync_prefill) {
                oPort_Sbuffers[i]->push_fill((AB_stype)0, bufsync_prefill - curLen);
              }
              prefillSyncState[i] = false;
            }
            oPort_Sbuffers[i]->pop_count(moveLen);
            continue;
          }
          if (curLen > bufsync_high) {
            oPort_Sbuffers[i]->pop_count(curLen - bufsync_prefill);
            oPort_bufavgpos_first[i] = (size_t)0;
            oPort_bufavgpos[i] = (size_t)0;
            oPort_bufavgpos_sum[i] = (long long int)0;
            oPort_bufavgpos_count[i] = (size_t)0;
            if (prefill_synccounts > (size_t)0) {
              oPort_Sbuffers[i]->pop_count(moveLen);
              prefillSyncState[i] = true;
            }
            else {
              oPort_Sbuffers[i]->pop_arr((AB_stype*)(out[i]), moveLen);
            }
            resyncH = (size_t)1;
            needParamUpdate = true;
            continue;
          }
          else if (curLen < bufsync_low + moveLen) {
            oPort_Sbuffers[i]->push_fill((AB_stype)0, bufsync_prefill + moveLen - curLen);
            oPort_bufavgpos_first[i] = (size_t)0;
            oPort_bufavgpos[i] = (size_t)0;
            oPort_bufavgpos_sum[i] = (long long int)0;
            oPort_bufavgpos_count[i] = (size_t)0;
            if (prefill_synccounts > (size_t)0) {
              oPort_Sbuffers[i]->pop_count(moveLen);
              prefillSyncState[i] = true;
            }
            else {
              oPort_Sbuffers[i]->pop_arr((AB_stype*)(out[i]), moveLen);
            }
            resyncL = (size_t)1;
            needParamUpdate = true;
            continue;
          }
          if (oPort_bufavgpos_first[i] > (size_t)0) {
            oPort_bufavgpos_sum[i] += (long long int)curLen;
            if (++(oPort_bufavgpos_count[i]) >= bufavgpos_counts) {
              oPort_bufavgpos[i] = (size_t)(oPort_bufavgpos_sum[i] / (long long int)(oPort_bufavgpos_count[i]));
              oPort_bufavgpos_sum[i] = (double)0.0;
              oPort_bufavgpos_count[i] = (size_t)0;
            }
            if (oPort_bufavgpos_first[i] + noresample_len < oPort_bufavgpos[i]) {
              oPort_Sbuffers[i]->pop_arr((AB_stype*)(out[i]), moveLen);
              oPort_Sbuffers[i]->pop();
              oPort_bufavgpos[i]--;
              resampleH = (size_t)1;
              needParamUpdate = true;
              continue;
            }
            else if (oPort_bufavgpos[i] + noresample_len < oPort_bufavgpos_first[i]) {
              oPort_Sbuffers[i]->pop_arr((AB_stype*)(out[i]), moveLen - (size_t)1);
              ((AB_stype*)(out[i]))[moveLen - (size_t)1] = oPort_Sbuffers[i]->read();
              oPort_bufavgpos[i]++;
              resampleL = (size_t)1;
              needParamUpdate = true;
              continue;
            }
          }
          else {
            oPort_bufavgpos_sum[i] += (long long int)curLen;
            if (++(oPort_bufavgpos_count[i]) >= bufavgpos_counts) {
              oPort_bufavgpos[i] = (size_t)(oPort_bufavgpos_sum[i] / (long long int)(oPort_bufavgpos_count[i]));
              oPort_bufavgpos_sum[i] = (double)0.0;
              oPort_bufavgpos_count[i] = (size_t)0;
              oPort_bufavgpos_first[i] = oPort_bufavgpos[i];
              needParamUpdate = true;
            }
          }
          oPort_Sbuffers[i]->pop_arr((AB_stype*)(out[i]), moveLen);
        }
        oPort_SharedM[0]->mem_unlock();
      }



    }
    else {
      //CASE OF 64bit in/out, or 32bit with no conversion.
      //CASE OF resample disabled.



      moveLen = bytes_len / sizeof(AB_stype);
      if (moveLen * sizeof(AB_stype) != bytes_len) {
        _32missing = (size_t)1;
        needParamUpdate = true;
      }

      //For input
      if (iPort_count > 0 && moveLen > (size_t)0) {
        iPort_SharedM[0]->mem_lock(SMEMORY_LOCK_INTERVAL);
        for (int i = 0; i < iCcount; i++) {
          emptyLen = iPort_Sbuffers[i]->get_curEmptyLen();
          if (moveLen > emptyLen) {
            iPort_Sbuffers[i]->pop_count(moveLen - emptyLen);
            overflow = (size_t)1;
            needParamUpdate = true;
          }
          else if (!iConnectionStarted) {
            _iConnectionStarted = true;
            needParamUpdate = true;
          }
          iPort_Sbuffers[i]->push_arr((AB_stype*)(in[i]), moveLen);
        }
        iPort_SharedM[0]->mem_unlock();
      }
      //For output
      if (oPort_count > 0 && moveLen > (size_t)0) {
        oPort_SharedM[0]->mem_lock(SMEMORY_LOCK_INTERVAL);
        for (int i = 0; i < oCcount; i++) {
          curLen = oPort_Sbuffers[i]->get_curLen();
          if (prefillSyncState[i]) {
            oPort_bufavgpos_sum[i] += (long long int)curLen;
            if (++(oPort_bufavgpos_count[i]) >= prefill_synccounts) {
              curLen = (size_t)(oPort_bufavgpos_sum[i] / (long long int)(oPort_bufavgpos_count[i]));
              oPort_bufavgpos_sum[i] = (double)0.0;
              oPort_bufavgpos_count[i] = (size_t)0;
              if (curLen > bufsync_prefill) {
                oPort_Sbuffers[i]->pop_count(curLen - bufsync_prefill);
              }
              else if (curLen < bufsync_prefill) {
                oPort_Sbuffers[i]->push_fill((AB_stype)0, bufsync_prefill - curLen);
              }
              prefillSyncState[i] = false;
            }
            oPort_Sbuffers[i]->pop_count(moveLen);
            continue;
          }
          if (curLen > bufsync_high) {
            oPort_Sbuffers[i]->pop_count(curLen - bufsync_prefill);
            oPort_bufavgpos_first[i] = (size_t)0;
            oPort_bufavgpos[i] = (size_t)0;
            oPort_bufavgpos_sum[i] = (long long int)0;
            oPort_bufavgpos_count[i] = (size_t)0;
            if (prefill_synccounts > (size_t)0) {
              oPort_Sbuffers[i]->pop_count(moveLen);
              prefillSyncState[i] = true;
            }
            else {
              oPort_Sbuffers[i]->pop_arr((AB_stype*)(out[i]), moveLen);
            }
            resyncH = (size_t)1;
            needParamUpdate = true;
            continue;
          }
          else if (curLen < bufsync_low + moveLen) {
            oPort_Sbuffers[i]->push_fill((AB_stype)0, bufsync_prefill + moveLen - curLen);
            oPort_bufavgpos_first[i] = (size_t)0;
            oPort_bufavgpos[i] = (size_t)0;
            oPort_bufavgpos_sum[i] = (long long int)0;
            oPort_bufavgpos_count[i] = (size_t)0;
            if (prefill_synccounts > (size_t)0) {
              oPort_Sbuffers[i]->pop_count(moveLen);
              prefillSyncState[i] = true;
            }
            else {
              oPort_Sbuffers[i]->pop_arr((AB_stype*)(out[i]), moveLen);
            }
            resyncL = (size_t)1;
            needParamUpdate = true;
            continue;
          }
          oPort_Sbuffers[i]->pop_arr((AB_stype*)(out[i]), moveLen);
        }
        oPort_SharedM[0]->mem_unlock();
      }



    }
  }
  else {
    if (bufavgpos_counts > (size_t)0) {
      //CASE OF 32bit with 64bit conversion.
      //CASE OF resample enabled.



      moveLen = bytes_len / sizeof(Vst::Sample32);

      //For input
      if (iPort_count > 0 && moveLen > (size_t)0) {
        iPort_SharedM[0]->mem_lock(SMEMORY_LOCK_INTERVAL);
        for (int i = 0; i < iCcount; i++) {
          emptyLen = iPort_Sbuffers[i]->get_curEmptyLen();
          if (moveLen > emptyLen) {
            iPort_Sbuffers[i]->pop_count(moveLen - emptyLen);
            overflow = (size_t)1;
            needParamUpdate = true;
          }
          else if (!iConnectionStarted) {
            _iConnectionStarted = true;
            needParamUpdate = true;
          }
          for (size_t j = (size_t)0; j < moveLen; j++) {
            iPort_Sbuffers[i]->push(
              (AB_stype)
              (((Vst::Sample32*)(in[i]))[j])
            );
          }
        }
        iPort_SharedM[0]->mem_unlock();
      }
      //For output
      if (oPort_count > 0 && moveLen > (size_t)0) {
        oPort_SharedM[0]->mem_lock(SMEMORY_LOCK_INTERVAL);
        for (int i = 0; i < oCcount; i++) {
          curLen = oPort_Sbuffers[i]->get_curLen();
          if (prefillSyncState[i]) {
            oPort_bufavgpos_sum[i] += (long long int)curLen;
            if (++(oPort_bufavgpos_count[i]) >= prefill_synccounts) {
              curLen = (size_t)(oPort_bufavgpos_sum[i] / (long long int)(oPort_bufavgpos_count[i]));
              oPort_bufavgpos_sum[i] = (double)0.0;
              oPort_bufavgpos_count[i] = (size_t)0;
              if (curLen > bufsync_prefill) {
                oPort_Sbuffers[i]->pop_count(curLen - bufsync_prefill);
              }
              else if (curLen < bufsync_prefill) {
                oPort_Sbuffers[i]->push_fill((AB_stype)0, bufsync_prefill - curLen);
              }
              prefillSyncState[i] = false;
            }
            oPort_Sbuffers[i]->pop_count(moveLen);
            continue;
          }
          if (curLen > bufsync_high) {
            oPort_Sbuffers[i]->pop_count(curLen - bufsync_prefill);
            oPort_bufavgpos_first[i] = (size_t)0;
            oPort_bufavgpos[i] = (size_t)0;
            oPort_bufavgpos_sum[i] = (long long int)0;
            oPort_bufavgpos_count[i] = (size_t)0;
            if (prefill_synccounts > (size_t)0) {
              oPort_Sbuffers[i]->pop_count(moveLen);
              prefillSyncState[i] = true;
            }
            else {
              for (size_t j = (size_t)0; j < moveLen; j++) {
                ((Vst::Sample32*)(out[i]))[j] = (Vst::Sample32)(oPort_Sbuffers[i]->pop());
              }
            }
            resyncH = (size_t)1;
            needParamUpdate = true;
            continue;
          }
          else if (curLen < bufsync_low + moveLen) {
            oPort_Sbuffers[i]->push_fill((AB_stype)0, bufsync_prefill + moveLen - curLen);
            oPort_bufavgpos_first[i] = (size_t)0;
            oPort_bufavgpos[i] = (size_t)0;
            oPort_bufavgpos_sum[i] = (long long int)0;
            oPort_bufavgpos_count[i] = (size_t)0;
            if (prefill_synccounts > (size_t)0) {
              oPort_Sbuffers[i]->pop_count(moveLen);
              prefillSyncState[i] = true;
            }
            else {
              for (size_t j = (size_t)0; j < moveLen; j++) {
                ((Vst::Sample32*)(out[i]))[j] = (Vst::Sample32)(oPort_Sbuffers[i]->pop());
              }
            }
            resyncL = (size_t)1;
            needParamUpdate = true;
            continue;
          }
          if (oPort_bufavgpos_first[i] > (size_t)0) {
            oPort_bufavgpos_sum[i] += (long long int)curLen;
            if (++(oPort_bufavgpos_count[i]) >= bufavgpos_counts) {
              oPort_bufavgpos[i] = (size_t)(oPort_bufavgpos_sum[i] / (long long int)(oPort_bufavgpos_count[i]));
              oPort_bufavgpos_sum[i] = (double)0.0;
              oPort_bufavgpos_count[i] = (size_t)0;
            }
            if (oPort_bufavgpos_first[i] + noresample_len < oPort_bufavgpos[i]) {
              for (size_t j = (size_t)0; j < moveLen; j++) {
                ((Vst::Sample32*)(out[i]))[j] = (Vst::Sample32)(oPort_Sbuffers[i]->pop());
              }
              oPort_Sbuffers[i]->pop();
              oPort_bufavgpos[i]--;
              resampleH = (size_t)1;
              needParamUpdate = true;
              continue;
            }
            else if (oPort_bufavgpos[i] + noresample_len < oPort_bufavgpos_first[i]) {
              for (size_t j = (size_t)0, jEnd = moveLen - (size_t)1; j < jEnd; j++) {
                ((Vst::Sample32*)(out[i]))[j] = (Vst::Sample32)(oPort_Sbuffers[i]->pop());
              }
              ((Vst::Sample32*)(out[i]))[moveLen - (size_t)1] = (Vst::Sample32)(oPort_Sbuffers[i]->read());
              oPort_bufavgpos[i]++;
              resampleL = (size_t)1;
              needParamUpdate = true;
              continue;
            }
          }
          else {
            oPort_bufavgpos_sum[i] += (long long int)curLen;
            if (++(oPort_bufavgpos_count[i]) >= bufavgpos_counts) {
              oPort_bufavgpos[i] = (size_t)(oPort_bufavgpos_sum[i] / (long long int)(oPort_bufavgpos_count[i]));
              oPort_bufavgpos_sum[i] = (double)0.0;
              oPort_bufavgpos_count[i] = (size_t)0;
              oPort_bufavgpos_first[i] = oPort_bufavgpos[i];
              needParamUpdate = true;
            }
          }
          for (size_t j = (size_t)0; j < moveLen; j++) {
            ((Vst::Sample32*)(out[i]))[j] = (Vst::Sample32)(oPort_Sbuffers[i]->pop());
          }
        }
        oPort_SharedM[0]->mem_unlock();
      }



    }
    else {
      //CASE OF 32bit with 64bit conversion.
      //CASE OF resample disabled.



      moveLen = bytes_len / sizeof(Vst::Sample32);

      //For input
      if (iPort_count > 0 && moveLen > (size_t)0) {
        iPort_SharedM[0]->mem_lock(SMEMORY_LOCK_INTERVAL);
        for (int i = 0; i < iCcount; i++) {
          emptyLen = iPort_Sbuffers[i]->get_curEmptyLen();
          if (moveLen > emptyLen) {
            iPort_Sbuffers[i]->pop_count(moveLen - emptyLen);
            overflow = (size_t)1;
            needParamUpdate = true;
          }
          else if (!iConnectionStarted) {
            _iConnectionStarted = true;
            needParamUpdate = true;
          }
          for (size_t j = (size_t)0; j < moveLen; j++) {
            iPort_Sbuffers[i]->push(
              (AB_stype)
              (((Vst::Sample32*)(in[i]))[j])
            );
          }
        }
        iPort_SharedM[0]->mem_unlock();
      }
      //For output
      if (oPort_count > 0 && moveLen > (size_t)0) {
        oPort_SharedM[0]->mem_lock(SMEMORY_LOCK_INTERVAL);
        for (int i = 0; i < oCcount; i++) {
          curLen = oPort_Sbuffers[i]->get_curLen();
          if (prefillSyncState[i]) {
            oPort_bufavgpos_sum[i] += (long long int)curLen;
            if (++(oPort_bufavgpos_count[i]) >= prefill_synccounts) {
              curLen = (size_t)(oPort_bufavgpos_sum[i] / (long long int)(oPort_bufavgpos_count[i]));
              oPort_bufavgpos_sum[i] = (double)0.0;
              oPort_bufavgpos_count[i] = (size_t)0;
              if (curLen > bufsync_prefill) {
                oPort_Sbuffers[i]->pop_count(curLen - bufsync_prefill);
              }
              else if (curLen < bufsync_prefill) {
                oPort_Sbuffers[i]->push_fill((AB_stype)0, bufsync_prefill - curLen);
              }
              prefillSyncState[i] = false;
            }
            oPort_Sbuffers[i]->pop_count(moveLen);
            continue;
          }
          if (curLen > bufsync_high) {
            oPort_Sbuffers[i]->pop_count(curLen - bufsync_prefill);
            oPort_bufavgpos_first[i] = (size_t)0;
            oPort_bufavgpos[i] = (size_t)0;
            oPort_bufavgpos_sum[i] = (long long int)0;
            oPort_bufavgpos_count[i] = (size_t)0;
            if (prefill_synccounts > (size_t)0) {
              oPort_Sbuffers[i]->pop_count(moveLen);
              prefillSyncState[i] = true;
            }
            else {
              for (size_t j = (size_t)0; j < moveLen; j++) {
                ((Vst::Sample32*)(out[i]))[j] = (Vst::Sample32)(oPort_Sbuffers[i]->pop());
              }
            }
            resyncH = (size_t)1;
            needParamUpdate = true;
            continue;
          }
          else if (curLen < bufsync_low + moveLen) {
            oPort_Sbuffers[i]->push_fill((AB_stype)0, bufsync_prefill + moveLen - curLen);
            oPort_bufavgpos_first[i] = (size_t)0;
            oPort_bufavgpos[i] = (size_t)0;
            oPort_bufavgpos_sum[i] = (long long int)0;
            oPort_bufavgpos_count[i] = (size_t)0;
            if (prefill_synccounts > (size_t)0) {
              oPort_Sbuffers[i]->pop_count(moveLen);
              prefillSyncState[i] = true;
            }
            else {
              for (size_t j = (size_t)0; j < moveLen; j++) {
                ((Vst::Sample32*)(out[i]))[j] = (Vst::Sample32)(oPort_Sbuffers[i]->pop());
              }
            }
            resyncL = (size_t)1;
            needParamUpdate = true;
            continue;
          }
          for (size_t j = (size_t)0; j < moveLen; j++) {
            ((Vst::Sample32*)(out[i]))[j] = (Vst::Sample32)(oPort_Sbuffers[i]->pop());
          }
        }
        oPort_SharedM[0]->mem_unlock();
      }



    }
  }


  //Write parameter changes

  //
  if (needParamUpdate) {
    c_overflow += overflow;
    c_resync_low += resyncL;
    c_resync_high += resyncH;
    c_resample_low += resampleL;
    c_resample_high += resampleH;
    c_32missing += _32missing;
    if (!oConnectionStarted && resyncH) {
      oConnectionStarted = true;
      c_overflow = (size_t)0;
      c_resync_low = (size_t)0;
      c_resync_high = (size_t)0;
      c_resample_low = (size_t)0;
      c_resample_high = (size_t)0;
      c_32missing = (size_t)0;
    }
    if (_iConnectionStarted) {
      iConnectionStarted = true;
      c_overflow = (size_t)0;
      c_resync_low = (size_t)0;
      c_resync_high = (size_t)0;
      c_resample_low = (size_t)0;
      c_resample_high = (size_t)0;
      c_32missing = (size_t)0;
    }
    int32 idx;
    Vst::IParamValueQueue* pvq;
    pvq = data.outputParameterChanges->addParameterData(RPPID::S_STARTED, idx);
    pvq = data.outputParameterChanges->getParameterData(idx);
    if (pvq) {
      if (iConnectionStarted && oConnectionStarted) {
        double param = 1.0;
        if (oPort_count > 0
          && oPort_bufavgpos_first[0] > (size_t)0
          && oPort_bufavgpos_first[0] < (size_t)BUFFER_ETC_MAX_SIZE) {
          param = (Vst::ParamValue)(oPort_bufavgpos_first[0]) / (Vst::ParamValue)BUFFER_ETC_MAX_SIZE;
        }
        pvq->addPoint(0, (Vst::ParamValue)param, idx);
      }
      else {
        pvq->addPoint(0, (Vst::ParamValue)0.0, idx);
      }
    }
    pvq = data.outputParameterChanges->addParameterData(RPPID::S_OVERFLOW, idx);
    pvq = data.outputParameterChanges->getParameterData(idx);
    if (pvq) {
      pvq->addPoint(0, c_overflow / (Vst::ParamValue)STATISTIC_MAXCOUNT, idx);
    }
    pvq = data.outputParameterChanges->addParameterData(RPPID::S_RESYNC, idx);
    pvq = data.outputParameterChanges->getParameterData(idx);
    if (pvq) {
      intTemp = (int)(c_resync_low + c_resync_high * (size_t)(STATISTIC_MAXCOUNT + 1));
      pvq->addPoint(0, intTemp / (Vst::ParamValue)((STATISTIC_MAXCOUNT + 1) * (STATISTIC_MAXCOUNT + 1) - 1), idx);
    }
    pvq = data.outputParameterChanges->addParameterData(RPPID::S_RESAMPLE, idx);
    pvq = data.outputParameterChanges->getParameterData(idx);
    if (pvq) {
      intTemp = (int)(c_resample_low + c_resample_high * (size_t)(STATISTIC_MAXCOUNT + 1));
      pvq->addPoint(0, intTemp / (Vst::ParamValue)((STATISTIC_MAXCOUNT + 1) * (STATISTIC_MAXCOUNT + 1) - 1), idx);
    }
    pvq = data.outputParameterChanges->addParameterData(RPPID::S_32MISSING, idx);
    pvq = data.outputParameterChanges->getParameterData(idx);
    if (pvq) {
      pvq->addPoint(0, c_32missing / (Vst::ParamValue)STATISTIC_MAXCOUNT, idx);
    }
  }

  return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RoutePePeProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
  //--- called before any processing ----

  return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API RoutePePeProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
  // by default kSample32 is supported
  if (symbolicSampleSize == Vst::kSample32)
    return kResultTrue;

  // disable the following comment if your processing support kSample64
  //*
  if (symbolicSampleSize == Vst::kSample64)
    return kResultTrue;
  //*/

  return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RoutePePeProcessor::setState (IBStream* state)
{
  if (!state) return kResultFalse;

  //
  doUpdate = true;

  // called when we load a preset, the model has to be reloaded
  IBStreamer streamer (state, kLittleEndian);
  bool tempB = false;
  int32 int32temp;
  int64 int64temp;

  //
  do {
    //
    tempB = streamer.readBool(doUpdate_v);
    if (!tempB) break;
    //
    tempB = streamer.readBool(f_noCvt_v);
    if (!tempB) break;

    //
    tempB = streamer.readInt32(int32temp);
    if (!tempB) break;
    sharedMem_intname_v = (int)int32temp;
    //
    tempB = streamer.readInt32(int32temp);
    if (!tempB) break;
    iPort_count_v = (int)int32temp;
    //
    tempB = streamer.readInt32(int32temp);
    if (!tempB) break;
    oPort_count_v = (int)int32temp;

    //
    tempB = streamer.readInt64(int64temp);
    if (!tempB) break;
    bufSize_v = (size_t)int64temp;
    //
    tempB = streamer.readInt64(int64temp);
    if (!tempB) break;
    bufsync_prefill_v = (size_t)int64temp;
    //
    tempB = streamer.readInt64(int64temp);
    if (!tempB) break;
    bufsync_low_v = (size_t)int64temp;
    //
    tempB = streamer.readInt64(int64temp);
    if (!tempB) break;
    bufsync_high_v = (size_t)int64temp;
    //
    tempB = streamer.readInt64(int64temp);
    if (!tempB) break;
    prefill_synccounts_v = (size_t)int64temp;

    //
    tempB = streamer.readInt64(int64temp);
    if (!tempB) break;
    bufavgpos_counts_v = int64temp;
    //
    tempB = streamer.readInt64(int64temp);
    if (!tempB) break;
    noresample_len_v = int64temp;
  } while (false);
  //
  if (sharedMem_intname_v < 0) sharedMem_intname_v = 0;
  else if (sharedMem_intname_v > SMEMORY_INTNAME_MAX) sharedMem_intname_v = SMEMORY_INTNAME_MAX;
  if (iPort_count_v < 0) iPort_count_v = 0;
  else if (iPort_count_v > MAX_PORTS) iPort_count_v = MAX_PORTS;
  if (oPort_count_v < 0) oPort_count_v = 0;
  else if (oPort_count_v > MAX_PORTS) oPort_count_v = MAX_PORTS;
  //
  if (bufSize_v > (size_t)BUFFER_ETC_MAX_SIZE) bufSize_v = (size_t)BUFFER_ETC_MAX_SIZE;
  if (bufsync_prefill_v > (size_t)BUFFER_ETC_MAX_SIZE) bufsync_prefill_v = (size_t)BUFFER_ETC_MAX_SIZE;
  if (bufsync_low_v > (size_t)BUFFER_ETC_MAX_SIZE) bufsync_low_v = (size_t)BUFFER_ETC_MAX_SIZE;
  if (bufsync_high_v > (size_t)BUFFER_ETC_MAX_SIZE) bufsync_high_v = (size_t)BUFFER_ETC_MAX_SIZE;
  if (prefill_synccounts_v > (size_t)BUFFER_ETC_MAX_SIZE) prefill_synccounts_v = (size_t)BUFFER_ETC_MAX_SIZE;
  if (bufavgpos_counts_v > (size_t)BUFFER_ETC_MAX_SIZE) bufavgpos_counts_v = (size_t)BUFFER_ETC_MAX_SIZE;
  if (noresample_len_v > (size_t)BUFFER_ETC_MAX_SIZE) noresample_len_v = (size_t)BUFFER_ETC_MAX_SIZE;

  if (!tempB) {
    return kResultFalse;
  }
  return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RoutePePeProcessor::getState (IBStream* state)
{
  // here we need to save the model
  IBStreamer streamer (state, kLittleEndian);

  //
  streamer.writeBool(doUpdate_v);
  streamer.writeBool(f_noCvt_v);
  //
  streamer.writeInt32((int32)sharedMem_intname_v);
  streamer.writeInt32((int32)iPort_count_v);
  streamer.writeInt32((int32)oPort_count_v);
  //
  streamer.writeInt64((int64)bufSize_v);
  streamer.writeInt64((int64)bufsync_prefill_v);
  streamer.writeInt64((int64)bufsync_low_v);
  streamer.writeInt64((int64)bufsync_high_v);
  streamer.writeInt64((int64)prefill_synccounts_v);
  //
  streamer.writeInt64((int64)bufavgpos_counts_v);
  streamer.writeInt64((int64)noresample_len_v);

  return kResultOk;
}

//------------------------------------------------------------------------
} // namespace LumyLumyStudio
