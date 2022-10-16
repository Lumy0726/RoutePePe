//------------------------------------------------------------------------
// Copyright(c) 2022 LumyLumyStudio.
//------------------------------------------------------------------------

#include "myplugincontroller.h"

//#include "vstgui/plugin-bindings/vst3editor.h"
#include "base/source/fstreamer.h"
#include "AudioBuffer.h"
#include <string.h>
#include <math.h>

using namespace Steinberg;

namespace LumyLumyStudio {

//------------------------------------------------------------------------
// RoutePePeController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API RoutePePeController::initialize (FUnknown* context)
{
  // Here the Plug-in will be instanciated

  //---do not forget to call parent ------
  tresult result = EditControllerEx1::initialize (context);
  if (result != kResultOk)
  {
    return result;
  }

  // Here you could register some parameters
  //*
  parameters.addParameter(
    STR16("Reset trigger mode"),
    STR16(""),
    (Steinberg::int32)(0),
    (Steinberg::Vst::ParamValue)(0.0),
    (Steinberg::int32)(Vst::ParameterInfo::kNoFlags),
    RPPID::DO_UPDATE, 0
  );
  parameters.addParameter(
    STR16("'R)' means need to reset to apply---"),
    STR16("<No effect>"),
    (Steinberg::int32)(0),
    (Steinberg::Vst::ParamValue)(0.0),
    (Steinberg::int32)(Vst::ParameterInfo::kNoFlags),
    RPPID::DESC1, 0
  );
  parameters.addParameter(
    STR16("R)32bit sample handling (No effect for 64bit)"),
    STR16(""),
    (Steinberg::int32)(0),
    (Steinberg::Vst::ParamValue)(0.0),
    (Steinberg::int32)(Vst::ParameterInfo::kNoFlags),
    RPPID::F_NOCVT, 0
  );
  parameters.addParameter(
    STR16("R)Connection (shared memory) ID"),
    STR16(""),
    (Steinberg::int32)(0),
    (Steinberg::Vst::ParamValue)(0.0),
    (Steinberg::int32)(Vst::ParameterInfo::kNoFlags),
    RPPID::SMEM_ID, 0
  );
  parameters.addParameter(
    STR16("R)InputNumber (for route)"),
    STR16(""),
    (Steinberg::int32)(0),
    (Steinberg::Vst::ParamValue)(0.0),
    (Steinberg::int32)(Vst::ParameterInfo::kNoFlags),
    RPPID::ICOUNT, 0
  );
  parameters.addParameter(
    STR16("R)OutputNumber (should, same, opposite input)"),
    STR16(""),
    (Steinberg::int32)(0),
    (Steinberg::Vst::ParamValue)(0.0),
    (Steinberg::int32)(Vst::ParameterInfo::kNoFlags),
    RPPID::OCOUNT, 0
  );
  parameters.addParameter(
    STR16("R)Buffer size (No effect for connection guest)"),
    STR16(""),
    (Steinberg::int32)(0),
    (Steinberg::Vst::ParamValue)(0.0),
    (Steinberg::int32)(Vst::ParameterInfo::kNoFlags),
    RPPID::BUFSIZE, 0
  );
  parameters.addParameter(
    STR16("Buffer prefill (avg latency)"),
    STR16(""),
    (Steinberg::int32)(0),
    (Steinberg::Vst::ParamValue)(0.0),
    (Steinberg::int32)(Vst::ParameterInfo::kNoFlags),
    RPPID::BUFSYNC_PREFILL, 0
  );
  parameters.addParameter(
    STR16("Buffer resync low"),
    STR16(""),
    (Steinberg::int32)(0),
    (Steinberg::Vst::ParamValue)(0.0),
    (Steinberg::int32)(Vst::ParameterInfo::kNoFlags),
    RPPID::BUFSYNC_LOW, 0
  );
  parameters.addParameter(
    STR16("Buffer resync high"),
    STR16(""),
    (Steinberg::int32)(0),
    (Steinberg::Vst::ParamValue)(0.0),
    (Steinberg::int32)(Vst::ParameterInfo::kNoFlags),
    RPPID::BUFSYNC_HIGH, 0
  );
  parameters.addParameter(
    STR16("Counts to get avg buffer pos (ForResample)"),
    STR16(""),
    (Steinberg::int32)(0),
    (Steinberg::Vst::ParamValue)(0.0),
    (Steinberg::int32)(Vst::ParameterInfo::kNoFlags),
    RPPID::BUFAVGPOS_COUNTS, 0
  );
  parameters.addParameter(
    STR16("Resample if avg buffer pos moves over->"),
    STR16(""),
    (Steinberg::int32)(0),
    (Steinberg::Vst::ParamValue)(0.0),
    (Steinberg::int32)(Vst::ParameterInfo::kNoFlags),
    RPPID::NORESAMPLE_LEN, 0
  );
  parameters.addParameter(
    STR16("<STATISTIC> (might need UI refresh)---"),
    STR16("<No effect>"),
    (Steinberg::int32)(0),
    (Steinberg::Vst::ParamValue)(0.0),
    (Steinberg::int32)(Vst::ParameterInfo::kNoFlags),
    RPPID::DESC2, 0
  );
  parameters.addParameter(
    STR16("Connected, (first avg buffer pos)"),
    STR16(""),
    (Steinberg::int32)(0),
    (Steinberg::Vst::ParamValue)(0.0),
    (Steinberg::int32)(Vst::ParameterInfo::kNoFlags),
    RPPID::S_STARTED, 0
  );
  parameters.addParameter(
    STR16("Resync - (low, high)"),
    STR16(""),
    (Steinberg::int32)(0),
    (Steinberg::Vst::ParamValue)(0.0),
    (Steinberg::int32)(Vst::ParameterInfo::kNoFlags),
    RPPID::S_RESYNC, 0
  );
  parameters.addParameter(
    STR16("Resample - (low, high)"),
    STR16(""),
    (Steinberg::int32)(0),
    (Steinberg::Vst::ParamValue)(0.0),
    (Steinberg::int32)(Vst::ParameterInfo::kNoFlags),
    RPPID::S_RESAMPLE, 0
  );
  parameters.addParameter(
    STR16("Overflow"),
    STR16(""),
    (Steinberg::int32)(0),
    (Steinberg::Vst::ParamValue)(0.0),
    (Steinberg::int32)(Vst::ParameterInfo::kNoFlags),
    RPPID::S_OVERFLOW, 0
  );
  parameters.addParameter(
    STR16("32bit sample bind error (odd count)"),
    STR16(""),
    (Steinberg::int32)(0),
    (Steinberg::Vst::ParamValue)(0.0),
    (Steinberg::int32)(Vst::ParameterInfo::kNoFlags),
    RPPID::S_32MISSING, 0
  );
  //*/

  return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RoutePePeController::terminate ()
{
  // Here the Plug-in will be de-instanciated, last possibility to remove some memory!

  //---do not forget to call parent ------
  return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API RoutePePeController::setComponentState (IBStream* state)
{
  if (!state) return kResultFalse;

  // Here you get the state of the component (Processor part)
  IBStreamer streamer(state, kLittleEndian);
  Vst::Parameter* pp;
  bool tempB = false;
  bool readB;
  int32 int32temp;
  int64 int64temp;

  //
  do {

    //
    tempB = streamer.readBool(readB);
    pp = parameters.getParameter(RPPID::DO_UPDATE);
    if (!tempB || pp == NULL) break;
    pp->setNormalized((Vst::ParamValue)(readB ? 1.0 : 0.0));
    //
    tempB = streamer.readBool(readB);
    pp = parameters.getParameter(RPPID::F_NOCVT);
    if (!tempB || pp == NULL) break;
    pp->setNormalized((Vst::ParamValue)(readB ? 1.0 : 0.0));

    //
    tempB = streamer.readInt32(int32temp);
    pp = parameters.getParameter(RPPID::SMEM_ID);
    if (!tempB || pp == NULL) break;
    pp->setNormalized(int32temp / (Vst::ParamValue)SMEMORY_INTNAME_MAX);
    //
    tempB = streamer.readInt32(int32temp);
    pp = parameters.getParameter(RPPID::ICOUNT);
    if (!tempB || pp == NULL) break;
    pp->setNormalized(int32temp / (Vst::ParamValue)MAX_PORTS);
    //
    tempB = streamer.readInt32(int32temp);
    pp = parameters.getParameter(RPPID::OCOUNT);
    if (!tempB || pp == NULL) break;
    pp->setNormalized(int32temp / (Vst::ParamValue)MAX_PORTS);

    //
    tempB = streamer.readInt64(int64temp);
    pp = parameters.getParameter(RPPID::BUFSIZE);
    if (!tempB || pp == NULL) break;
    pp->setNormalized((Steinberg::Vst::ParamValue)((double)regularSize2int((int)int64temp, BUFFER_HALFSCALE) / (double)BUFFER_OPTION_COUNT));
    //
    tempB = streamer.readInt64(int64temp);
    pp = parameters.getParameter(RPPID::BUFSYNC_PREFILL);
    if (!tempB || pp == NULL) break;
    pp->setNormalized((Steinberg::Vst::ParamValue)((double)regularSize2int((int)int64temp, BUFFER_HALFSCALE) / (double)BUFFER_OPTION_COUNT));
    //
    tempB = streamer.readInt64(int64temp);
    pp = parameters.getParameter(RPPID::BUFSYNC_LOW);
    if (!tempB || pp == NULL) break;
    pp->setNormalized((Steinberg::Vst::ParamValue)((double)regularSize2int((int)int64temp, BUFFER_HALFSCALE) / (double)BUFFER_OPTION_COUNT));
    //
    tempB = streamer.readInt64(int64temp);
    pp = parameters.getParameter(RPPID::BUFSYNC_HIGH);
    if (!tempB || pp == NULL) break;
    pp->setNormalized((Steinberg::Vst::ParamValue)((double)regularSize2int((int)int64temp, BUFFER_HALFSCALE) / (double)BUFFER_OPTION_COUNT));

    //
    tempB = streamer.readInt64(int64temp);
    pp = parameters.getParameter(RPPID::BUFAVGPOS_COUNTS);
    if (!tempB || pp == NULL) break;
    pp->setNormalized((Steinberg::Vst::ParamValue)((double)regularSize2int((int)int64temp, BUFFER_HALFSCALE) / (double)BUFFER_OPTION_COUNT));
    //
    tempB = streamer.readInt64(int64temp);
    pp = parameters.getParameter(RPPID::NORESAMPLE_LEN);
    if (!tempB || pp == NULL) break;
    pp->setNormalized((Steinberg::Vst::ParamValue)((double)regularSize2int((int)int64temp, BUFFER_HALFSCALE) / (double)BUFFER_OPTION_COUNT));
  } while (false);

  if (!tempB) {
    return kResultFalse;
  }
  return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RoutePePeController::setState (IBStream* state)
{
  // Here you get the state of the controller

  return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RoutePePeController::getState (IBStream* state)
{
  // Here you are asked to deliver the state of the controller (if needed)
  // Note: the real state of your plug-in is saved in the processor

  return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API RoutePePeController::createView (FIDString name)
{
  // Here the Host wants to open your editor (if you have one)
  if (FIDStringsEqual (name, Vst::ViewType::kEditor))
  {
    // create your editor here and return a IPlugView ptr of it
        return nullptr;
  }
  return nullptr;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RoutePePeController::setParamNormalized (Vst::ParamID tag, Vst::ParamValue value)
{
  // called by host to update your parameters
  tresult result = EditControllerEx1::setParamNormalized (tag, value);
  return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RoutePePeController::getParamStringByValue (Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string)
{
  // called by host to get a string for given normalized value of a specific parameter
  // (without having to set the value!)

  int intTemp, intTemp2;
  switch (tag) {

    //
  case RPPID::DO_UPDATE:
    if (valueNormalized > (Vst::ParamValue)0.5) {
      wcscpy_s((wchar_t*)string, (rsize_t)128, L"Often (NotRecommended)");
    }
    else {
      wcscpy_s((wchar_t*)string, (rsize_t)128, L"Default");
    }
    return Steinberg::kResultOk;
  case RPPID::F_NOCVT:
    if (valueNormalized > (Vst::ParamValue)0.5) {
      wcscpy_s((wchar_t*)string, (rsize_t)128, L"Convert to 64bit (MoreCPU)");
    }
    else {
      wcscpy_s((wchar_t*)string, (rsize_t)128, L"Bind two 32bit as 64bit");
    }
    return Steinberg::kResultOk;
    //
  case RPPID::SMEM_ID:
    intTemp = (int)(::round((double)(valueNormalized) * SMEMORY_INTNAME_MAX));
    if (intTemp) {
      swprintf_s((wchar_t*)string, (size_t)128, L"ID:< %02d >", intTemp);
    }
    else {
      wcscpy_s((wchar_t*)string, (rsize_t)128, L"< NO CONNECTION ID >");
    }
    return Steinberg::kResultOk;
    //
  case RPPID::ICOUNT:
  case RPPID::OCOUNT:
    intTemp = (int)(::round((double)(valueNormalized) * MAX_PORTS));
    swprintf_s((wchar_t*)string, (size_t)128, L"%02d Channels", intTemp);
    return Steinberg::kResultOk;
    //
  case RPPID::BUFSIZE:
  case RPPID::BUFSYNC_PREFILL:
  case RPPID::BUFSYNC_LOW:
  case RPPID::BUFSYNC_HIGH:
    intTemp = (int)(::round((double)(valueNormalized) * (double)BUFFER_OPTION_COUNT));
    swprintf_s((wchar_t*)string, (size_t)128, L"%02d Samples", int2regularSize(intTemp, BUFFER_HALFSCALE));
    return Steinberg::kResultOk;
  case RPPID::BUFAVGPOS_COUNTS:
    intTemp = (int)(::round((double)(valueNormalized) * (double)BUFFER_OPTION_COUNT));
    if (intTemp) {
      swprintf_s((wchar_t*)string, (size_t)128, L"%02d Counts", int2regularSize(intTemp, BUFFER_HALFSCALE));
    }
    else {
      wcscpy_s((wchar_t*)string, (rsize_t)128, L"No resample");
    }
    return Steinberg::kResultOk;
  case RPPID::NORESAMPLE_LEN:
    intTemp = (int)(::round((double)(valueNormalized) * (double)BUFFER_OPTION_COUNT));
    swprintf_s((wchar_t*)string, (size_t)128, L"%02d (+/-) Samples", int2regularSize(intTemp, BUFFER_HALFSCALE));
    return Steinberg::kResultOk;
    //
  case RPPID::S_STARTED:
    if (valueNormalized == (Vst::ParamValue)0.0) {
      wcscpy_s((wchar_t*)string, (rsize_t)128, L"false, (?? Samples)");
    }
    else if (valueNormalized == (Vst::ParamValue)1.0) {
      wcscpy_s((wchar_t*)string, (rsize_t)128, L"true, (?? Samples)");
    }
    else {
      intTemp = (int)(::round((double)(valueNormalized) * (double)BUFFER_ETC_MAX_SIZE));
      swprintf_s((wchar_t*)string, (size_t)128, L"true, (%04d Samples)", intTemp);
    }
    return Steinberg::kResultOk;
  case RPPID::S_RESYNC:
  case RPPID::S_RESAMPLE:
    intTemp = (int)(::round((double)(valueNormalized) * (
      (STATISTIC_MAXCOUNT + 1) * (STATISTIC_MAXCOUNT + 1) - 1
      )));
    intTemp2 = intTemp % (STATISTIC_MAXCOUNT + 1);
    intTemp = intTemp / (STATISTIC_MAXCOUNT + 1);
    swprintf_s((wchar_t*)string, (size_t)128,
      L"(%d%s, %d%s)",
      intTemp2,
      (intTemp >= STATISTIC_MAXCOUNT) ? L"+" : L"",
      intTemp,
      (intTemp >= STATISTIC_MAXCOUNT) ? L"+" : L""
    );
    return Steinberg::kResultOk;
  case RPPID::S_OVERFLOW:
  case RPPID::S_32MISSING:
    intTemp = (int)(::round((double)(valueNormalized) * STATISTIC_MAXCOUNT));
    if (intTemp >= STATISTIC_MAXCOUNT) {
      swprintf_s((wchar_t*)string, (size_t)128, L"%d+", intTemp);
    }
    else {
      swprintf_s((wchar_t*)string, (size_t)128, L"%d", intTemp);
    }
    return Steinberg::kResultOk;
  }

  return EditControllerEx1::getParamStringByValue(tag, valueNormalized, string);
}

//------------------------------------------------------------------------
tresult PLUGIN_API RoutePePeController::getParamValueByString (Vst::ParamID tag, Vst::TChar* string, Vst::ParamValue& valueNormalized)
{
  // called by host to get a normalized value from a string representation of a specific parameter
  // (without having to set the value!)

  int intTemp, intTemp2;
  switch (tag) {

    //
  case RPPID::DO_UPDATE:
    if ((wchar_t)(string[0]) == L'O') {
      valueNormalized = (Vst::ParamValue)(1.0);
    }
    else {
      valueNormalized = (Vst::ParamValue)(0.0);
    }
    return Steinberg::kResultOk;
  case RPPID::F_NOCVT:
    if ((wchar_t)(string[0]) == L'C') {
      valueNormalized = (Vst::ParamValue)(1.0);
    }
    else {
      valueNormalized = (Vst::ParamValue)(0.0);
    }
    return Steinberg::kResultOk;
    //
  case RPPID::SMEM_ID:
    if ((wchar_t)(string[0]) == L'I'
      && (wchar_t)(string[1]) == L'D'
      && (wchar_t)(string[2]) == L':'
      && (wchar_t)(string[3]) == L'<'
      && (wchar_t)(string[4]) == L' ') {
      intTemp = _wtoi((const wchar_t*)string + 5);
      valueNormalized = intTemp / (Vst::ParamValue)SMEMORY_INTNAME_MAX;
    }
    else {
      valueNormalized = (Vst::ParamValue)(0.0);
    }
    return Steinberg::kResultOk;
    //
  case RPPID::ICOUNT:
  case RPPID::OCOUNT:
    intTemp = _wtoi((const wchar_t*)string);
    valueNormalized = intTemp / (Vst::ParamValue)MAX_PORTS;
    return Steinberg::kResultOk;
    //
  case RPPID::BUFSIZE:
  case RPPID::BUFSYNC_PREFILL:
  case RPPID::BUFSYNC_LOW:
  case RPPID::BUFSYNC_HIGH:
  case RPPID::NORESAMPLE_LEN:
    intTemp = _wtoi((const wchar_t*)string);
    valueNormalized = (Steinberg::Vst::ParamValue)((double)regularSize2int(intTemp, BUFFER_HALFSCALE) / (double)BUFFER_OPTION_COUNT);
    return Steinberg::kResultOk;
  case RPPID::BUFAVGPOS_COUNTS:
    if ((wchar_t)(string[0]) == L'N') {
      valueNormalized = (Vst::ParamValue)(0.0);
    }
    else {
      intTemp = _wtoi((const wchar_t*)string);
      valueNormalized = (Steinberg::Vst::ParamValue)((double)regularSize2int(intTemp, BUFFER_HALFSCALE) / (double)BUFFER_OPTION_COUNT);
    }
    return Steinberg::kResultOk;
    //
  case RPPID::S_STARTED:
    if ((wchar_t)(string[0]) == L't'
      && (wchar_t)(string[1]) == L'r'
      && (wchar_t)(string[2]) == L'u'
      && (wchar_t)(string[3]) == L'e'
      && (wchar_t)(string[4]) == L','
      && (wchar_t)(string[5]) == L' '
      && (wchar_t)(string[6]) == L'(') {
      intTemp = _wtoi((const wchar_t*)string + 7);
      if (intTemp > 0) {
        valueNormalized = (Vst::ParamValue)intTemp / (Vst::ParamValue)BUFFER_ETC_MAX_SIZE;
      }
      else {
        valueNormalized = (Vst::ParamValue)(1.0);
      }
    }
    if ((wchar_t)(string[0]) == L't') {
      valueNormalized = (Vst::ParamValue)(1.0);
    }
    else {
      valueNormalized = (Vst::ParamValue)(0.0);
    }
    return Steinberg::kResultOk;
  case RPPID::S_RESYNC:
  case RPPID::S_RESAMPLE:
    intTemp = intTemp2 = 0;
    for (size_t i = (size_t)0; true; ) {
      wchar_t w = ((const wchar_t*)string)[i++];
      if (w == (wchar_t)0) break;
      if (w == L'(') {
        intTemp2 = _wtoi((const wchar_t*)string + i);
      }
      if (w == L' ') {
        intTemp = _wtoi((const wchar_t*)string + i);
      }
    }
    intTemp = intTemp2 + intTemp * (STATISTIC_MAXCOUNT + 1);
    valueNormalized = intTemp / (Vst::ParamValue)((STATISTIC_MAXCOUNT + 1) * (STATISTIC_MAXCOUNT + 1) - 1);
    return Steinberg::kResultOk;
  case RPPID::S_OVERFLOW:
  case RPPID::S_32MISSING:
    intTemp = _wtoi((const wchar_t*)string);
    valueNormalized = intTemp / (Vst::ParamValue)STATISTIC_MAXCOUNT;
    return Steinberg::kResultOk;
  }

  return EditControllerEx1::getParamValueByString (tag, string, valueNormalized);
}

//------------------------------------------------------------------------
} // namespace LumyLumyStudio
