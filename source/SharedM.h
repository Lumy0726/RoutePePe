//------------------------------------------------------------------------
// Copyright(c) 2022 LumyLumyStudio.
//------------------------------------------------------------------------

#pragma once
#include <thread>
#include <mutex>
#include <Windows.h>


namespace LumyLumyStudio {


//-------------------------
//type declaration
//-------------------------

/*
SharedM.
Manage Win32 shared memory between process.
.
NOTE:
  This class ITSELF is not shared memory.
  This class ITSELT is not synchronized automatically.
  For multi thread, use other synchronous method manually.
  This class is not designed for inheritance.
*/
class SharedM {
public:
  //-------------------------
  //typedef of SharedM
  //-------------------------



  //-------------------------
  //field of SharedM
  //-------------------------

  //
protected:
  HANDLE mem_handle;
  void* mem_allocAddr;
  void* mem_contAddr;
  size_t mem_allocSize;
  size_t mem_contSize;
public:



  //-------------------------
  //function of SharedM
  //-------------------------

  //constructor
  SharedM();
  SharedM(const SharedM& input) = delete;
  //destructor
  ~SharedM();
  //assign operator
  SharedM& operator=(const SharedM& input) = delete;
  //comparator - by memory address, not actual data.
  bool operator==(const SharedM& input) const;
  //comparator - by memory address, not actual data.
  bool operator<(const SharedM& input) const;
  //comparator - by memory address, not actual data.
  bool operator>(const SharedM& input) const;
  //comparator - by memory address, not actual data.
  bool operator<=(const SharedM& input) const;
  //comparator - by memory address, not actual data.
  bool operator>=(const SharedM& input) const;



  //-------------------------
  //function of SharedM - getter
  //-------------------------

  //
protected:
  HANDLE get_mem_handle() const { return mem_handle; }
  void* get_mem_allocAddr() const { return mem_allocAddr; }
  size_t get_mem_allocSize() const { return mem_allocSize; }
public:
  void* get_mem_contAddr() const { return mem_contAddr; }
  size_t get_mem_contSize() const { return mem_contSize; }



  //-------------------------
  //function of SharedM - creation etc
  //-------------------------

  /*
  @brief  init_mem.
          If shared memory is already opened by this class, do nothing, return false.
          If "name" is NULL, do nothing, return false.
  */
  bool init_mem(const char* name, size_t contSize);
  /*
  @brief  open_mem.
          If shared memory is already opened by this class, do nothing, return false.
          If "name" is NULL, do nothing, return false.
          "contSize" value after success is,
            may be bigger than actual shared memory size first created,
            but generally it has no problem read/write that memory area.
  */
  bool open_mem(const char* name);
  /*
  @brief  close_mem.
          This does not mean that destroying the shared memory, rather, stop to use shared memory.
          Shared memory will be automatically destroyed if there is no one using memory.
          If there is no shared memory, do nothing.
  */
  void close_mem();



  //-------------------------
  //function of SharedM - for synchronize
  //-------------------------

  /*
  @brief  Try to lock shared memory.
          This guarantee memory lock between process(thread) that read/write shared memory with lock.
          This does not guarantee actual memory lock,
            any process(thread) that read/write without lock can cause problem.
  */
  bool mem_tryLock();
  /*
  @brief  Lock shared memory.
          This guarantee memory lock between process(thread) that read/write shared memory with lock.
          This does not guarantee actual memory lock,
            any process(thread) that read/write without lock can cause problem.
  @param  tryInterval - wait interval, microseconds. Value 0 cause busy loop. Default is 1.
  */
  void mem_lock(int tryInterval = 1);
  /*
  @brief  Wait for signal of calling "mem_unlock", and lock shared memory.
          To catch the signal, "mem_unlock" signal of other process(thread) should happen AFTER calling this method.
          This method is useful to confirm that shared memory does not contain un-initialized (trash) value,
            at the process(thread) that open shared memory (not create).
          This guarantee memory lock between process(thread) that read/write shared memory with lock.
          This does not guarantee actual memory lock,
            any process(thread) that read/write without lock can cause problem.
  @param  tryInterval - wait interval, microseconds. Value 0 cause busy loop. Default is 1.
  */
  void mem_lock_catchUnlock(int tryInterval = 1);
  /*
  @brief  Wait for signal of calling "mem_unlock".
          To catch the signal, "mem_unlock" signal of other process(thread) should happen AFTER calling this method.
          This method is useful to confirm that shared memory does not contain un-initialized (trash) value,
            at the process(thread) that open shared memory (not create).
          This does not lock shared memory automatically.
  @param  try_count - max count to try to get signal. If this is 0, no count limit applied. Default is 0.
  @param  tryInterval - wait interval, microseconds. Value 0 cause busy loop. Default is 1.
  @return true, if there was signal.
  */
  bool mem_catchUnlock(size_t try_count = 0, int tryInterval = 1);
  /*
  @brief  Unlock shared memory.
          Calling this function from process(thread) that has no lock, cause undefined behavior.
  */
  void mem_unlock();
};



//------------------------------------------------------------------------
} // namespace LumyLumyStudio


//EOF