//------------------------------------------------------------------------
// Copyright(c) 2022 LumyLumyStudio.
//------------------------------------------------------------------------

#pragma once
#include "SharedM.h"


namespace LumyLumyStudio {


//-------------------------
//function of SharedM
//-------------------------

//constructor
SharedM::SharedM() {
  //
  mem_handle = NULL;
  mem_allocAddr = NULL;
  mem_contAddr = NULL;
  mem_allocSize = (size_t)0;
  mem_contSize = (size_t)0;
}
//destructor
SharedM::~SharedM() {
  if (mem_handle != NULL) {
    UnmapViewOfFile((LPCVOID)mem_allocAddr);
    CloseHandle(mem_handle);
  }
}
//comparator
bool SharedM::operator==(const SharedM& input) const {
  return this == &input;
}
bool SharedM::operator<(const SharedM& input) const {
  return this < &input;
}
bool SharedM::operator>(const SharedM& input) const {
  return this > &input;
}
bool SharedM::operator<=(const SharedM& input) const {
  return this <= &input;
}
bool SharedM::operator>=(const SharedM& input) const {
  return this >= &input;
}



//-------------------------
//function of SharedM - creation etc
//-------------------------

//
bool SharedM::init_mem(const char* name, size_t contSize) {
  if (mem_handle) return false;
  if (name == NULL) return false;
  if (contSize <= 0) return false;
  size_t allocSize = contSize + (sizeof(int) * (size_t)2);
  if (allocSize <= (sizeof(int) * (size_t)2)) return false;
  DWORD allocSize_low = (DWORD)allocSize;
  size_t allocSize_high = allocSize;
  for (size_t i = (size_t)0; i < sizeof(DWORD); i++) {
    allocSize_high = allocSize_high / 256;
  }
  if (allocSize_high != (size_t)((DWORD)allocSize_high)) return false;
  //

  SetLastError(NO_ERROR);
  mem_handle = CreateFileMappingA(
    INVALID_HANDLE_VALUE,
    NULL,
    PAGE_READWRITE,
    (DWORD)allocSize_high,
    allocSize_low,
    (LPCSTR)name
  );
  if (mem_handle == NULL) return false;
  if (mem_handle == INVALID_HANDLE_VALUE) {
    mem_handle = NULL;
    return false;
  }
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    CloseHandle(mem_handle);
    mem_handle = NULL;
    return false;
  }
  mem_allocAddr = MapViewOfFile(
    mem_handle,
    FILE_MAP_READ | FILE_MAP_WRITE,
    0,
    0,
    (SIZE_T)allocSize
  );
  if (mem_allocAddr == NULL) {
    CloseHandle(mem_handle);
    mem_handle = NULL;
    return false;
  }
  mem_contAddr = (char*)(mem_allocAddr) + (sizeof(int) * (size_t)2);
  ((int*)mem_allocAddr)[0] = 0;
  //
  mem_allocSize = allocSize;
  mem_contSize = contSize;
  //
  return true;
}
bool SharedM::open_mem(const char* name) {
  if (mem_handle) return false;
  if (name == NULL) return false;

  //
  mem_handle = OpenFileMappingA(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, (LPCSTR)name);
  if (mem_handle == NULL) return false;
  mem_allocAddr = MapViewOfFile(
    mem_handle,
    FILE_MAP_READ | FILE_MAP_WRITE,
    0,
    0,
    (SIZE_T)0
  );
  if (mem_allocAddr == NULL) {
    CloseHandle(mem_handle);
    mem_handle = NULL;
    return false;
  }
  MEMORY_BASIC_INFORMATION mInfo;
  mem_allocSize = (size_t)VirtualQueryEx(GetCurrentProcess(), mem_allocAddr, &mInfo, sizeof(mInfo));
  if (mem_allocSize > (size_t)0) mem_allocSize = mInfo.RegionSize;
  if (mem_allocSize <= (sizeof(int) * (size_t)2)) {
    UnmapViewOfFile(mem_allocAddr);
    CloseHandle(mem_handle);
    mem_allocSize = (size_t)0;
    mem_allocAddr = NULL;
    mem_handle = NULL;
    return false;
  }
  //
  mem_contAddr = (char*)(mem_allocAddr)+(sizeof(int) * (size_t)2);
  mem_contSize = mem_allocSize - (sizeof(int) * (size_t)2);
  //
  return true;
}
void SharedM::close_mem() {
  if (mem_handle != NULL) {
    UnmapViewOfFile((LPCVOID)mem_allocAddr);
    CloseHandle(mem_handle);
  }
  //
  mem_handle = NULL;
  mem_allocAddr = NULL;
  mem_contAddr = NULL;
  mem_allocSize = (size_t)0;
  mem_contSize = (size_t)0;
}



//-------------------------
//function of SharedM - for synchronize
//-------------------------

//
bool SharedM::mem_tryLock() {
  unsigned int retLock = InterlockedExchange(
    (volatile unsigned int*)mem_allocAddr,
    (unsigned int)1);
  return retLock != (unsigned int)1;
}
void SharedM::mem_lock(int tryInterval) {
  unsigned int retLock;
  if (tryInterval > 0) {
    while (true) {
      retLock = InterlockedExchange(
        (volatile unsigned int*)mem_allocAddr,
        (unsigned int)1);
      if (retLock != (unsigned int)1) break;
      std::this_thread::sleep_for(std::chrono::microseconds(tryInterval));
    }
  }
  else {
    while (true) {
      retLock = InterlockedExchange(
        (volatile unsigned int*)mem_allocAddr,
        (unsigned int)1);
      if (retLock != (unsigned int)1) break;
    }
  }
}
void SharedM::mem_lock_catchUnlock(int tryInterval) {
  unsigned int unlockCounter;
  //
  unlockCounter = InterlockedCompareExchange(
    (volatile unsigned int*)((unsigned int*)mem_allocAddr + 1),
    (unsigned int)0,
    (unsigned int)0
  );
  //
  if (tryInterval > 0) {
    while (true) {
      std::this_thread::sleep_for(std::chrono::microseconds(tryInterval));
      unsigned int uc = InterlockedCompareExchange(
        (volatile unsigned int*)((unsigned int*)mem_allocAddr + 1),
        (unsigned int)0,
        (unsigned int)0
      );
      if (uc != unlockCounter) break;
    }
  }
  else {
    while (true) {
      unsigned int uc = InterlockedCompareExchange(
        (volatile unsigned int*)((unsigned int*)mem_allocAddr + 1),
        (unsigned int)0,
        (unsigned int)0
      );
      if (uc != unlockCounter) break;
    }
  }
  //
  mem_lock(tryInterval);
}
bool SharedM::mem_catchUnlock(size_t try_count, int tryInterval) {
  unsigned int unlockCounter;
  //
  unlockCounter = InterlockedCompareExchange(
    (volatile unsigned int*)((unsigned int*)mem_allocAddr + 1),
    (unsigned int)0,
    (unsigned int)0
  );
  //
  if (tryInterval > 0) {
    while (true) {
      std::this_thread::sleep_for(std::chrono::microseconds(tryInterval));
      unsigned int uc = InterlockedCompareExchange(
        (volatile unsigned int*)((unsigned int*)mem_allocAddr + 1),
        (unsigned int)0,
        (unsigned int)0
      );
      if (uc != unlockCounter) break;
      if (try_count == (size_t)1) { return false; }
      else if (try_count > (size_t)1) { try_count--; }
    }
  }
  else {
    while (true) {
      unsigned int uc = InterlockedCompareExchange(
        (volatile unsigned int*)((unsigned int*)mem_allocAddr + 1),
        (unsigned int)0,
        (unsigned int)0
      );
      if (uc != unlockCounter) break;
      if (try_count == (size_t)1) { return false; }
      else if (try_count > (size_t)1) { try_count--; }
    }
  }
  return true;
}
void SharedM::mem_unlock() {
  ((unsigned int*)mem_allocAddr)[1]++;
  InterlockedExchange(
    (volatile unsigned int*)mem_allocAddr,
    (unsigned int)0);
}



//------------------------------------------------------------------------
} // namespace LumyLumyStudio


//EOF