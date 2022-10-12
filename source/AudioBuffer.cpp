//------------------------------------------------------------------------
// Copyright(c) 2022 LumyLumyStudio.
//------------------------------------------------------------------------

#include "AudioBuffer.h"


namespace LumyLumyStudio {


//-------------------------
//global function declaration
//-------------------------

//

int int2regularSize(int i, int halfscale) {
  if (i <= 0) return 0;
  int ret = 2;
  if ((i % 2) == 0) { ret = 3; }
  int loop = (i - 1) / 2;
  while (--loop >= 0) {
    ret = ret << 1;
  }
  return ret * halfscale;
}
int regularSize2int(int i, int halfscale) {
  i = i / halfscale;
  if (i <= 1) return 0;
  int q, r, c = 0;
  while (true) {
    q = i / 2;
    r = i % 4;
    if (q <= 1) {
      break;
    }
    i = q;
    c++;
  }
  return r - 1 + 2 * c;
}


//-------------------------
//function of AudioBuffer
//-------------------------

//constructor
AudioBuffer::AudioBuffer() {
  //
  buf = NULL;
  buf_allocSize = (size_t)0;
  buf_contSize = (size_t)0;
  curLen = (size_t)0;
  rpos = (size_t)0;
  wpos = (size_t)0;
}
AudioBuffer::AudioBuffer(const AudioBuffer& input) {
  //
  if (input.buf) {
    buf = NULL;
    buf = new AB_stype[input.buf_allocSize];
    memcpy(buf, input.buf, sizeof(AB_stype) * (input.buf_allocSize));
  }
  else {
    buf = NULL;
  }
  buf_allocSize = input.buf_allocSize;
  buf_contSize = input.buf_contSize;
  curLen = input.curLen;
  rpos = input.rpos;
  wpos = input.wpos;
}
AudioBuffer::AudioBuffer(AudioBuffer&& input) noexcept {
  buf = std::move(input.buf);
  buf_allocSize = std::move(input.buf_allocSize);
  buf_contSize = std::move(input.buf_contSize);
  curLen = std::move(input.curLen);
  rpos = std::move(input.rpos);
  wpos = std::move(input.wpos);
}
//destructor
AudioBuffer::~AudioBuffer() {
  if (buf) delete[] buf;
}
//assign operator
AudioBuffer& AudioBuffer::operator=(const AudioBuffer& input) {
  if (this != &input) {
    //
    if (buf) delete[] buf;
    if (input.buf) {
      buf = NULL;
      buf = new AB_stype[input.buf_allocSize];
      memcpy(buf, input.buf, sizeof(AB_stype) * (input.buf_allocSize));
    }
    else {
      buf = NULL;
    }
    buf_allocSize = input.buf_allocSize;
    buf_contSize = input.buf_contSize;
    curLen = input.curLen;
    rpos = input.rpos;
    wpos = input.wpos;
  }
  return *this;
}
AudioBuffer& AudioBuffer::operator=(AudioBuffer&& input) noexcept {
  if (this != &input) {
    //
    buf = std::move(input.buf);
    buf_allocSize = std::move(input.buf_allocSize);
    buf_contSize = std::move(input.buf_contSize);
    curLen = std::move(input.curLen);
    rpos = std::move(input.rpos);
    wpos = std::move(input.wpos);
  }
  return *this;
}
//comparator
bool AudioBuffer::operator==(const AudioBuffer& input) const {
  return this == &input;
}
bool AudioBuffer::operator<(const AudioBuffer& input) const {
  return this < &input;
}
bool AudioBuffer::operator>(const AudioBuffer& input) const {
  return this > & input;
}
bool AudioBuffer::operator<=(const AudioBuffer& input) const {
  return this <= &input;
}
bool AudioBuffer::operator>=(const AudioBuffer& input) const {
  return this >= &input;
}



//-------------------------
//function of AudioBuffer - i/o
//-------------------------

//
void AudioBuffer::init_buf(size_t contSize) {
  if (buf) return;
  if (contSize <= 0) return;
  size_t allocSize = contSize + (size_t)1;
  if (allocSize <= 0) return;
  //
  buf = new AB_stype[allocSize];
  buf_allocSize = allocSize;
  buf_contSize = contSize;
  curLen = (size_t)0;
  rpos = (size_t)0;
  wpos = (size_t)0;
}
void AudioBuffer::delete_buf() {
  if (buf) delete[] buf;
  buf_allocSize = (size_t)0;
  buf_contSize = (size_t)0;
  curLen = (size_t)0;
  rpos = (size_t)0;
  wpos = (size_t)0;
}
//
void AudioBuffer::push(AB_stype value) {
  if (curLen < buf_contSize) {
    buf[wpos] = value;
    wpos = (wpos + (size_t)1) % buf_allocSize;
    curLen++;
  }
}
void AudioBuffer::push_arr(const AB_stype* arr, size_t len) {
  size_t _len = buf_contSize - curLen;
  if (len < _len) _len = len;
  size_t wpos1 = wpos + _len;
  if (buf_allocSize < wpos1) {
    memcpy(
      buf + wpos,
      arr,
      sizeof(AB_stype) * (buf_allocSize - wpos));
    memcpy(
      buf,
      arr + (buf_allocSize - wpos),
      sizeof(AB_stype) * (wpos1 - buf_allocSize));
  }
  else {
    memcpy(buf + wpos, arr, sizeof(AB_stype) * (_len));
  }
  wpos = (wpos1) % buf_allocSize;
  curLen += _len;
}
void AudioBuffer::push_fill(AB_stype value, size_t len) {
  while (curLen < buf_contSize) {
    buf[wpos] = value;
    wpos = (wpos + (size_t)1) % buf_allocSize;
    curLen++;
    if (len == (size_t)1) { break; }
    else if (len > (size_t)1) { len--; }
  }
}
AB_stype AudioBuffer::pop() {
  AB_stype ret = (AB_stype)0;
  if (curLen > (size_t)0) {
    ret = buf[rpos];
    rpos = (rpos + (size_t)1) % buf_allocSize;
    curLen--;
  }
  return ret;
}
void AudioBuffer::pop_arr(AB_stype* arr, size_t len) {
  size_t _len = curLen;
  if (len < _len) _len = len;
  size_t rpos1 = rpos + _len;
  if (buf_allocSize < rpos1) {
    memcpy(
      arr,
      buf + rpos,
      sizeof(AB_stype) * (buf_allocSize - rpos));
    memcpy(
      arr + (buf_allocSize - rpos),
      buf,
      sizeof(AB_stype) * (rpos1 - buf_allocSize));
  }
  else {
    memcpy(arr, buf + rpos, sizeof(AB_stype) * (_len));
  }
  rpos = (rpos1) % buf_allocSize;
  curLen -= _len;
}
void AudioBuffer::pop_count(size_t len) {
  size_t _len = curLen;
  if (len < _len) _len = len;
  rpos = (rpos + _len) % buf_allocSize;
  curLen -= _len;
}
AB_stype AudioBuffer::read() {
  AB_stype ret = (AB_stype)0;
  if (curLen > (size_t)0) {
    ret = buf[rpos];
  }
  return ret;
}
void AudioBuffer::read_arr(
  size_t len,
  const AB_stype** arr1_out, size_t* len1_out,
  const AB_stype** arr2_out, size_t* len2_out)
{
  size_t _len = curLen;
  if (len < _len) _len = len;
  size_t rpos1 = rpos + _len;
  if (buf_allocSize < rpos1) {
    *arr1_out = buf + rpos;
    *len1_out = (buf_allocSize - rpos);
    *arr2_out = buf;
    *len2_out = (rpos1 - buf_allocSize);
  }
  else {
    *arr1_out = buf + rpos;
    *len1_out = _len;
    *arr2_out = buf;
    *len2_out = (size_t)0;
  }
}



//-------------------------
//function of AudioBufferOnMEM
//-------------------------

//constructor
AudioBufferOnMEM::AudioBufferOnMEM() {
  //
  buf = NULL;
  buf_end_secure = NULL;
  p_buf_allocSize = NULL;
  p_buf_contSize = NULL;
  p_curLen = NULL;
  p_rpos = NULL;
  p_wpos = NULL;
}
//destructor
AudioBufferOnMEM::~AudioBufferOnMEM() {
  ;
}
//comparator
bool AudioBufferOnMEM::operator==(const AudioBufferOnMEM& input) const {
  return this == &input;
}
bool AudioBufferOnMEM::operator<(const AudioBufferOnMEM& input) const {
  return this < &input;
}
bool AudioBufferOnMEM::operator>(const AudioBufferOnMEM& input) const {
  return this > &input;
}
bool AudioBufferOnMEM::operator<=(const AudioBufferOnMEM& input) const {
  return this <= &input;
}
bool AudioBufferOnMEM::operator>=(const AudioBufferOnMEM& input) const {
  return this >= &input;
}



//-------------------------
//function of AudioBufferOnMEM - i/o
//-------------------------

//
bool AudioBufferOnMEM::init_class(void* mem_addr, size_t mem_size) {
  if (buf) return false;
  if (mem_addr == NULL) return false;
  size_t allocSize;
  //
  //init address
  p_buf_allocSize = (size_t*)mem_addr;
  p_buf_contSize = p_buf_allocSize + 1;
  p_curLen = p_buf_contSize + 1;
  p_rpos = p_curLen + 1;
  p_wpos = p_rpos + 1;
  buf = (AB_stype*)(p_wpos + 1);
  //
  //size error check
  if ((void*)(buf + 2) > (void*)((char*)mem_addr + mem_size)) {
    //CASE OF not enough "mem_size".
    buf = NULL;
    p_buf_allocSize = NULL;
    p_buf_contSize = NULL;
    p_curLen = NULL;
    p_rpos = NULL;
    p_wpos = NULL;
    return false;
  }
  //
  //init value
  allocSize = (((char*)mem_addr + mem_size) - (char*)buf) / sizeof(AB_stype);
  *p_buf_allocSize = allocSize;
  buf_end_secure = buf + allocSize;//do not put "*p_buf_allocSize" instead of "allocSize", for secure.
  *p_buf_contSize = *p_buf_allocSize - (size_t)1;
  *p_curLen = (size_t)0;
  *p_rpos = (size_t)0;
  *p_wpos = (size_t)0;
  return true;
}
void AudioBufferOnMEM::load_class(void* mem_addr, size_t mem_size) {
  if (buf) return;
  if (mem_addr == NULL) return;
  //
  //load address
  p_buf_allocSize = (size_t*)mem_addr;
  p_buf_contSize = p_buf_allocSize + 1;
  p_curLen = p_buf_contSize + 1;
  p_rpos = p_curLen + 1;
  p_wpos = p_rpos + 1;
  buf = (AB_stype*)(p_wpos + 1);
  size_t allocSize_secure = (((char*)mem_addr + mem_size) - (char*)buf) / sizeof(AB_stype);
  buf_end_secure = buf + allocSize_secure;
}
//
void AudioBufferOnMEM::push(AB_stype value) {
  if (*p_curLen < *p_buf_contSize) {
    size_t _wpos = *p_wpos;
    if (buf + _wpos < buf_end_secure) { //for secure
      buf[_wpos] = value;
    }
    *p_wpos = (*p_wpos + (size_t)1) % *p_buf_allocSize;
    (*p_curLen)++;
  }
}
void AudioBufferOnMEM::push_arr(const AB_stype* arr, size_t len) {
  size_t _len = *p_buf_contSize - *p_curLen;
  if (len < _len) _len = len;
  size_t _allocSize = *p_buf_allocSize;
  size_t _wpos = *p_wpos;
  size_t wpos1 = _wpos + _len;
  if (buf + _allocSize <= buf_end_secure && _wpos < _allocSize && _len < _allocSize) { //for secure
    if (_allocSize < wpos1) {
      memcpy(
        buf + _wpos,
        arr,
        sizeof(AB_stype) * (_allocSize - _wpos));
      memcpy(
        buf,
        arr + (_allocSize - _wpos),
        sizeof(AB_stype) * (wpos1 - _allocSize));
    }
    else {
      memcpy(buf + _wpos, arr, sizeof(AB_stype) * (_len));
    }
  }
  *p_wpos = (wpos1) % *p_buf_allocSize;
  *p_curLen += _len;
}
void AudioBufferOnMEM::push_fill(AB_stype value, size_t len) {
  size_t _wpos = *p_wpos;
  while (*p_curLen < *p_buf_contSize) {
    if (buf + _wpos < buf_end_secure) { //for secure
      buf[_wpos] = value;
    }
    _wpos = (_wpos + (size_t)1) % *p_buf_allocSize;
    (*p_curLen)++;
    if (len == (size_t)1) { break; }
    else if (len > (size_t)1) { len--; }
  }
  *p_wpos = _wpos;
}
AB_stype AudioBufferOnMEM::pop() {
  AB_stype ret = (AB_stype)0;
  if (*p_curLen > (size_t)0) {
    size_t _rpos = *p_rpos;
    if (buf + _rpos < buf_end_secure) { //for secure
      ret = buf[_rpos];
    }
    *p_rpos = (*p_rpos + (size_t)1) % *p_buf_allocSize;
    (*p_curLen)--;
  }
  return ret;
}
void AudioBufferOnMEM::pop_arr(AB_stype* arr, size_t len) {
  size_t _len = *p_curLen;
  if (len < _len) _len = len;
  size_t _allocSize = *p_buf_allocSize;
  size_t _rpos = *p_rpos;
  size_t rpos1 = _rpos + _len;
  if (buf + _allocSize <= buf_end_secure && _rpos < _allocSize && _len < _allocSize) { //for secure
    if (_allocSize < rpos1) {
      memcpy(
        arr,
        buf + _rpos,
        sizeof(AB_stype) * (_allocSize - _rpos));
      memcpy(
        arr + (_allocSize - _rpos),
        buf,
        sizeof(AB_stype) * (rpos1 - _allocSize));
    }
    else {
      memcpy(arr, buf + _rpos, sizeof(AB_stype) * (_len));
    }
  }
  *p_rpos = (rpos1) % *p_buf_allocSize;
  *p_curLen -= _len;
}
void AudioBufferOnMEM::pop_count(size_t len) {
  size_t _len = *p_curLen;
  if (len < _len) _len = len;
  *p_rpos = (*p_rpos + _len) % *p_buf_allocSize;
  *p_curLen -= _len;
}
AB_stype AudioBufferOnMEM::read() {
  AB_stype ret = (AB_stype)0;
  if (*p_curLen > (size_t)0) {
    size_t _rpos = *p_rpos;
    if (buf + _rpos < buf_end_secure) { //for secure
      ret = buf[_rpos];
    }
  }
  return ret;
}
void AudioBufferOnMEM::read_arr(
  size_t len,
  const AB_stype** arr1_out, size_t* len1_out,
  const AB_stype** arr2_out, size_t* len2_out)
{
  size_t _len = *p_curLen;
  if (len < _len) _len = len;
  size_t _allocSize = *p_buf_allocSize;
  size_t _rpos = *p_rpos;
  size_t rpos1 = _rpos + _len;
  if (buf + _allocSize <= buf_end_secure && _rpos < _allocSize && _len < _allocSize) { //for secure
    if (_allocSize < rpos1) {
      *arr1_out = buf + _rpos;
      *len1_out = (_allocSize - _rpos);
      *arr2_out = buf;
      *len2_out = (rpos1 - _allocSize);
    }
    else {
      *arr1_out = buf + _rpos;
      *len1_out = _len;
      *arr2_out = buf;
      *len2_out = (size_t)0;
    }
  }
  else {
    *arr1_out = buf;
    *len1_out = (size_t)0;
    *arr2_out = buf;
    *len2_out = (size_t)0;
  }
}




//------------------------------------------------------------------------
} // namespace LumyLumyStudio


//EOF