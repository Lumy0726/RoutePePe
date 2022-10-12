//------------------------------------------------------------------------
// Copyright(c) 2022 LumyLumyStudio.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"


namespace LumyLumyStudio {

//-------------------------
//global function declaration
//-------------------------

//for i (1, 3, 5, 7, ...) return "halfscalue" * (2, 4, 8, 16, ...)
//for i (2, 4, 6, 8, ...) return "halfscalue" * (3, 6, 12, 24, ...)
//for i is 0, return 0
int int2regularSize(int i, int halfscale = 32);
//inverse of "int2regularSize".
//for example, if 'i' is 8 return 5, if 'i' is 12 return 6, 'i' between them return 5.
int regularSize2int(int i, int halfscale = 32);

//-------------------------
//type declaration
//-------------------------

//
typedef Steinberg::Vst::Sample64 AB_stype;

/*
AudioBuffer.
Custom audioBuffer for RoutePePe VST.
.
NOTE:
  This class is not synchronized automatically.
  For multi thread, use other synchronous method manually.
  This class is not designed for inheritance.
*/
class AudioBuffer {
public:
  //-------------------------
  //typedef of AudioBuffer
  //-------------------------



  //-------------------------
  //field of AudioBuffer
  //-------------------------

protected:
  //
  AB_stype* buf;
  size_t buf_allocSize;
  size_t buf_contSize;
  size_t curLen;
  size_t rpos;
  size_t wpos;
public:



  //-------------------------
  //function of AudioBuffer
  //-------------------------

  //constructor
  AudioBuffer();
  AudioBuffer(const AudioBuffer& input);
  AudioBuffer(AudioBuffer&& input) noexcept;
  //destructor
  ~AudioBuffer();
  //assign operator
  AudioBuffer& operator=(const AudioBuffer& input);
  AudioBuffer& operator=(AudioBuffer&& input) noexcept;
  //comparator - by memory address, not actual data.
  bool operator==(const AudioBuffer& input) const;
  //comparator - by memory address, not actual data.
  bool operator<(const AudioBuffer& input) const;
  //comparator - by memory address, not actual data.
  bool operator>(const AudioBuffer& input) const;
  //comparator - by memory address, not actual data.
  bool operator<=(const AudioBuffer& input) const;
  //comparator - by memory address, not actual data.
  bool operator>=(const AudioBuffer& input) const;



  //-------------------------
  //function of AudioBuffer - getter
  //-------------------------

  //
  const AB_stype* get_buf() const { return buf; }
  size_t get_buf_allocSize() const { return buf_allocSize; }
  size_t get_buf_contSize() const { return buf_contSize; }
  size_t get_curLen() const { return curLen; }
  size_t get_curEmptyLen() const { return buf_contSize - curLen; }
  size_t get_rpos() const { return rpos; }
  size_t get_wpos() const { return wpos; }



  //-------------------------
  //function of AudioBuffer - i/o
  //-------------------------

  /*
  @brief  init_buf. If buffer is already allocated, do nothing.
  */
  void init_buf(size_t contSize);
  /*
  @brief  delete_buf. If buffer is not allocated, do nothing.
  */
  void delete_buf();

  /*
  @brief  push. If there is no empty area, do nothing.
  */
  void push(AB_stype value);
  /*
  @brief  push_arr. If there is no enough empty area, just push as much as possible.
  */
  void push_arr(const AB_stype* arr, size_t len);
  /*
  @brief  push_fill.
          If there is no enough empty area, just push as much as possible.
          If "len" is 0, fill all empty buffer.
  */
  void push_fill(AB_stype value, size_t len);
  /*
  @brief  pop. If there is no data, return 0.
  */
  AB_stype pop();
  /*
  @brief  pop_arr. If there is no enough data to read, just read as much as possible (rest area of "arr" value would be unchanged).
          Note that this moves data,
            so it is recommended to use "read_arr" and "pop_count",
            to get more performance.
  */
  void pop_arr(AB_stype* arr, size_t len);
  /*
  @brief  pop_count.
          If there is no enough data to pop, just pop as much as possible.
          If "len" is 0, do nothing.
  */
  void pop_count(size_t len);
  /*
  @brief  read. If there is no data, return 0.
  */
  AB_stype read();
  /*
  @brief  read_arr. If there is no enough data to read, just read as much as possible.
          Data from "~out" value will be invalid if this class would be modified.
  */
  void read_arr(
    size_t len,
    const AB_stype** arr1_out, size_t* len1_out,
    const AB_stype** arr2_out, size_t* len2_out);
};

/*
AudioBufferOnMEM.
Custom audioBuffer for RoutePePe VST.
All data related with audio buffer is on target memory.
.
NOTE:
  This class is not synchronized automatically.
  For multi thread, use other synchronous method manually.
  This class is not designed for inheritance.
*/
class AudioBufferOnMEM {
public:
  //-------------------------
  //typedef of AudioBufferOnMEM
  //-------------------------



  //-------------------------
  //field of AudioBufferOnMEM, (and related simple things etc)
  //-------------------------

protected:
  //
  AB_stype* buf;
  AB_stype* buf_end_secure;
  size_t* p_buf_allocSize;
  size_t* p_buf_contSize;
  size_t* p_curLen;
  size_t* p_rpos;
  size_t* p_wpos;
public:
  static const size_t MIN_MEM_SIZE = sizeof(size_t) * (size_t)5 + sizeof(AB_stype) * (size_t)2;
  static size_t bufLen2memSize(size_t bufLen) {
    return sizeof(size_t) * (size_t)5 + sizeof(AB_stype) * (bufLen + (size_t)1);
  }



  //-------------------------
  //function of AudioBufferOnMEM
  //-------------------------

  //constructor
  AudioBufferOnMEM();
  AudioBufferOnMEM(const AudioBufferOnMEM& input) = delete;
  //destructor
  ~AudioBufferOnMEM();
  //assign operator
  AudioBufferOnMEM& operator=(const AudioBufferOnMEM& input) = delete;
  //comparator - by memory address, not actual data.
  bool operator==(const AudioBufferOnMEM& input) const;
  //comparator - by memory address, not actual data.
  bool operator<(const AudioBufferOnMEM& input) const;
  //comparator - by memory address, not actual data.
  bool operator>(const AudioBufferOnMEM& input) const;
  //comparator - by memory address, not actual data.
  bool operator<=(const AudioBufferOnMEM& input) const;
  //comparator - by memory address, not actual data.
  bool operator>=(const AudioBufferOnMEM& input) const;



  //-------------------------
  //function of AudioBufferOnMEM - getter
  //-------------------------

  //
  const AB_stype* get_buf() const { return buf; }
  const AB_stype* get_buf_end_secure() const { return buf_end_secure; }
  size_t get_buf_allocSize() const { return *p_buf_allocSize; }
  size_t get_buf_contSize() const { return *p_buf_contSize; }
  size_t get_curLen() const { return *p_curLen; }
  size_t get_curEmptyLen() const { return *p_buf_contSize - *p_curLen; }
  size_t get_rpos() const { return *p_rpos; }
  size_t get_wpos() const { return *p_wpos; }



  //-------------------------
  //function of AudioBufferOnMEM - i/o
  //-------------------------

  /*
  @brief  Init class, at empty memory.
          If class was already initialized, do nothing, return false.
          "mem_size" should be enough (at least MIN_MEM_SIZE), otherwise return false.
  */
  bool init_class(void* mem_addr, size_t mem_size);
  /*
  @brief  Load class, from memory.
          If class was already initialized, do nothing.
          Note that this function never read/write inside memory (use only address value).
          "mem_size" is for secure purpose,
            to prevent AudioBufferOnMEM access outside of memory,
            when memory is corrupted by other things.
  */
  void load_class(void* mem_addr, size_t mem_size);

  /*
  @brief  push. If there is no empty area, do nothing.
  */
  void push(AB_stype value);
  /*
  @brief  push_arr. If there is no enough empty area, just push as much as possible.
  */
  void push_arr(const AB_stype* arr, size_t len);
  /*
  @brief  push_fill.
          If there is no enough empty area, just push as much as possible.
          If "len" is 0, fill all empty buffer.
  */
  void push_fill(AB_stype value, size_t len);
  /*
  @brief  pop. If there is no data, return 0.
  */
  AB_stype pop();
  /*
  @brief  pop_arr. If there is no enough data to read, just read as much as possible (rest area of "arr" value would be unchanged).
          Note that this moves data,
            so it is recommended to use "read_arr" and "pop_count",
            to get more performance.
  */
  void pop_arr(AB_stype* arr, size_t len);
  /*
  @brief  pop_count.
          If there is no enough data to pop, just pop as much as possible.
          If "len" is 0, do nothing.
  */
  void pop_count(size_t len);
  /*
  @brief  read. If there is no data, return 0.
  */
  AB_stype read();
  /*
  @brief  read_arr. If there is no enough data to read, just read as much as possible.
          Data from "~out" value will be invalid if this class would be modified.
  */
  void read_arr(
    size_t len,
    const AB_stype** arr1_out, size_t* len1_out,
    const AB_stype** arr2_out, size_t* len2_out);
};



//------------------------------------------------------------------------
} // namespace LumyLumyStudio


//EOF