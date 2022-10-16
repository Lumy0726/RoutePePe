//------------------------------------------------------------------------
// Copyright(c) 2022 LumyLumyStudio.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace LumyLumyStudio {
//------------------------------------------------------------------------
static const Steinberg::FUID kRoutePePeProcessorUID (0xBF3711BF, 0xE6DB5EBA, 0xB87DB50C, 0x6C00C989);
static const Steinberg::FUID kRoutePePeControllerUID (0x8FBFB5D5, 0xD17E5BDD, 0xAE7AC123, 0x8B659B7A);

#define RoutePePeVST3Category "Fx"

//-------------------------
//MACRO
//-------------------------

#define MAX_PORTS 32
#define BUFFER_ETC_MAX_SIZE 0x00ffffff
#define BUFFER_HALFSCALE 8
#define BUFFER_OPTION_COUNT 38
#define SMEMORY_LOCK_INTERVAL 1
#define SMEMORY_INTNAME_MAX 100
#define STATISTIC_MAXCOUNT 1000

enum RPPID : Steinberg::Vst::ParamID {
  DO_UPDATE = 201,
  F_NOCVT = 202,
  SMEM_ID = 203,
  ICOUNT = 204,
  OCOUNT = 205,
  BUFSIZE = 206,
  BUFSYNC_PREFILL = 207,
  BUFSYNC_LOW = 208,
  BUFSYNC_HIGH = 209,
  BUFAVGPOS_COUNTS = 210,
  NORESAMPLE_LEN = 211,

  S_STARTED = 250,
  S_RESYNC = 251,
  S_RESAMPLE = 252,
  S_OVERFLOW = 253,
  S_32MISSING = 254,

  DESC1 = 301,
  DESC2 = 302,
  //DESC3 = 303,
};

//------------------------------------------------------------------------
} // namespace LumyLumyStudio
