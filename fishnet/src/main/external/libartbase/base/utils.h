/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ART_LIBARTBASE_BASE_UTILS_H_
#define ART_LIBARTBASE_BASE_UTILS_H_

#include <pthread.h>
#include <stdlib.h>

#include <random>
#include <string>

#include <android-base/logging.h>
#include <android-base/parseint.h>

#include "casts.h"
#include "globals.h"
#include "macros.h"
#include "pointer_size.h"

#if defined(__linux__)
#include <sys/utsname.h>
#endif

namespace art {

static inline uint32_t PointerToLowMemUInt32(const void* p) {
  uintptr_t intp = reinterpret_cast<uintptr_t>(p);
  DCHECK_LE(intp, 0xFFFFFFFFU);
  return intp & 0xFFFFFFFFU;
}

// Returns a human-readable size string such as "1MB".
std::string PrettySize(uint64_t size_in_bytes);

// Splits a string using the given separator character into a vector of
// strings. Empty strings will be omitted.
template<typename StrIn, typename Str>
void Split(const StrIn& s, char separator, std::vector<Str>* out_result);

template<typename Str>
void Split(const Str& s, char separator, size_t len, Str* out_result);

template<typename StrIn, typename Str, size_t kLen>
void Split(const StrIn& s, char separator, std::array<Str, kLen>* out_result) {
  Split<Str>(Str(s), separator, kLen, &((*out_result)[0]));
}

// Returns the calling thread's tid. (The C libraries don't expose this.)
uint32_t GetTid();

// Returns the given thread's name.
std::string GetThreadName(pid_t tid);

// Sets the pthread name of the current thread. The name may be truncated to an
// implementation-defined limit.
void SetThreadName(const char* thread_name);

// Sets the pthread name of the given thread. The name may be truncated to an
// implementation-defined limit. Does nothing if not supported by the OS.
void SetThreadName(pthread_t thr, const char* thread_name);

// Reads data from "/proc/self/task/${tid}/stat".
void GetTaskStats(pid_t tid, char* state, int* utime, int* stime, int* task_cpu);

class VoidFunctor {
 public:
  template <typename A>
  inline void operator()([[maybe_unused]] A a) const {}

  template <typename A, typename B>
  inline void operator()([[maybe_unused]] A a, [[maybe_unused]] B b) const {}

  template <typename A, typename B, typename C>
  inline void operator()([[maybe_unused]] A a, [[maybe_unused]] B b, [[maybe_unused]] C c) const {}
};

static inline const void* EntryPointToCodePointer(const void* entry_point) {
  uintptr_t code = reinterpret_cast<uintptr_t>(entry_point);
  // TODO: Make this Thumb2 specific. It is benign on other architectures as code is always at
  //       least 2 byte aligned.
  code &= ~0x1;
  return reinterpret_cast<const void*>(code);
}

#if defined(__BIONIC__)
struct Arc4RandomGenerator {
  using result_type = uint32_t;
  static constexpr uint32_t min() { return std::numeric_limits<uint32_t>::min(); }
  static constexpr uint32_t max() { return std::numeric_limits<uint32_t>::max(); }
  uint32_t operator() () { return arc4random(); }
};
using RNG = Arc4RandomGenerator;
#else
using RNG = std::random_device;
#endif

template <typename T>
static T GetRandomNumber(T min, T max) {
  CHECK_LT(min, max);
  std::uniform_int_distribution<T> dist(min, max);
  RNG rng;
  return dist(rng);
}

// Sleep forever and never come back.
NO_RETURN void SleepForever();

// Flush CPU caches. Returns true on success, false if flush failed.
WARN_UNUSED bool FlushCpuCaches(void* begin, void* end);

#if defined(__linux__)
bool IsKernelVersionAtLeast(int reqd_major, int reqd_minor);
#endif

// On some old kernels, a cache operation may segfault.
WARN_UNUSED bool CacheOperationsMaySegFault();

// Is the execution environment on a virtual machine? See ART_TEST_ON_VM.
WARN_UNUSED bool RunningOnVM();

template <typename Func, typename... Args>
static inline void CheckedCall(const Func& function, const char* what, Args... args) {
  int rc = function(args...);
  if (UNLIKELY(rc != 0)) {
    PLOG(FATAL) << "Checked call failed for " << what;
  }
}

// Forces the compiler to emit a load instruction, but discards the value.
// Useful when dealing with memory paging.
template <typename T>
inline void ForceRead(const T* pointer) {
  static_cast<void>(*const_cast<volatile T*>(pointer));
}

// Lookup value for a given key in /proc/self/status. Keys and values are separated by a ':' in
// the status file. Returns value found on success and "<unknown>" if the key is not found or
// there is an I/O error.
std::string GetProcessStatus(const char* key);

// Copy a prefix of /proc/tid/stat of the given length into buf. Return the number of bytes
// actually read, 0 on error.
size_t GetOsThreadStat(pid_t tid, char* buf, size_t len);

// Return a short prefix of /proc/tid/stat as quickly and robustly as possible. Used for debugging
// timing issues and possibly issues with /proc itself. Always atomic.
std::string GetOsThreadStatQuick(pid_t tid);

// Return a concatenation of the output of GetOsThreadStatQuick(tid) for all other tids.
// Less robust against concurrent change, but individual stat strings should still always
// be consistent. Called only when we are nearly certain to crash anyway.
std::string GetOtherThreadOsStats();

}  // namespace art

#endif  // ART_LIBARTBASE_BASE_UTILS_H_
