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

#ifndef ART_LIBARTBASE_BASE_LEB128_H_
#define ART_LIBARTBASE_BASE_LEB128_H_

#include <optional>
#include <vector>

#include <android-base/logging.h>

#include "bit_utils.h"
#include "globals.h"
#include "macros.h"

namespace art {

template <typename T>
static inline bool DecodeLeb128Helper(const uint8_t** data,
                                      const std::optional<const void*>& end,
                                      T* out) {
  static_assert(sizeof(T) == 8 || sizeof(T) == 4);
  const uint8_t* ptr = *data;
  T result = 0;
  const size_t num_bits = (sizeof(T) * 8);
  // We can encode 7-bits per byte in leb128. So max_bytes is ceil(number_of_bits / 7)
  const size_t max_bytes = (num_bits + 6u) / 7u;
  for (size_t index = 0; index < max_bytes; ++index) {
    if (end.has_value() && ptr >= end.value()) {
      return false;
    }

    std::make_unsigned_t<T> curr = *(ptr++);
    result |= ((curr & 0x7f) << (index * 7));
    if (LIKELY(curr <= 0x7f)) {
      if (std::is_signed_v<T>) {
        // For signed values we need to sign extend the result. If we are using all the bits then
        // the result is already sign extended and we don't need to do anything.
        if (index < max_bytes - 1) {
          int shift = num_bits - (index + 1) * 7;
          result = (result << shift) >> shift;
        }
      }
      // End of encoding.
      break;
    }
  }

  *out = result;
  *data = ptr;
  return true;
}

template <typename T = uint32_t>
static inline T DecodeUnsignedLeb128(const uint8_t** data) {
  static_assert(!std::is_signed_v<T>);
  T value = 0;
  DecodeLeb128Helper(data, std::nullopt, &value);
  return value;
}

template <typename T = uint32_t>
static inline bool DecodeUnsignedLeb128Checked(const uint8_t** data, const void* end, T* out) {
  static_assert(!std::is_signed_v<T>);
  return DecodeLeb128Helper(data, end, out);
}

static inline uint32_t DecodeUnsignedLeb128WithoutMovingCursor(const uint8_t* data) {
  return DecodeUnsignedLeb128(&data);
}

// Reads an unsigned LEB128 + 1 value. updating the given pointer to point
// just past the end of the read value. This function tolerates
// non-zero high-order bits in the fifth encoded byte.
// It is possible for this function to return -1.
static inline int32_t DecodeUnsignedLeb128P1(const uint8_t** data) {
  return DecodeUnsignedLeb128(data) - 1;
}

template <typename T = int32_t>
static inline T DecodeSignedLeb128(const uint8_t** data) {
  static_assert(std::is_signed_v<T>);
  T value = 0;
  DecodeLeb128Helper(data, std::nullopt, &value);
  return value;
}

template <typename T = int32_t>
static inline bool DecodeSignedLeb128Checked(const uint8_t** data, const void* end, T* out) {
  static_assert(std::is_signed_v<T>);
  return DecodeLeb128Helper(data, end, out);
}

// Returns the number of bytes needed to encode the value in unsigned LEB128.
static inline uint32_t UnsignedLeb128Size(uint64_t data) {
  // bits_to_encode = (data != 0) ? 64 - CLZ(x) : 1  // 64 - CLZ(data | 1)
  // bytes = ceil(bits_to_encode / 7.0);             // (6 + bits_to_encode) / 7
  uint32_t x = 6 + 64 - CLZ(data | 1U);
  // Division by 7 is done by (x * 37) >> 8 where 37 = ceil(256 / 7).
  // This works for 0 <= x < 256 / (7 * 37 - 256), i.e. 0 <= x <= 85.
  return (x * 37) >> 8;
}

static inline bool IsLeb128Terminator(const uint8_t* ptr) {
  return *ptr <= 0x7f;
}

// Returns the first byte of a Leb128 value assuming that:
// (1) `end_ptr` points to the first byte after the Leb128 value, and
// (2) there is another Leb128 value before this one.
template <typename T>
static inline T* ReverseSearchUnsignedLeb128(T* end_ptr) {
  static_assert(std::is_same_v<std::remove_const_t<T>, uint8_t>,
                "T must be a uint8_t");
  T* ptr = end_ptr;

  // Move one byte back, check that this is the terminating byte.
  ptr--;
  DCHECK(IsLeb128Terminator(ptr));

  // Keep moving back while the previous byte is not a terminating byte.
  // Fail after reading five bytes in case there isn't another Leb128 value
  // before this one.
  while (!IsLeb128Terminator(ptr - 1)) {
    ptr--;
    DCHECK_LE(static_cast<ptrdiff_t>(end_ptr - ptr), 5);
  }

  return ptr;
}

// Returns the number of bytes needed to encode the value in unsigned LEB128.
static inline uint32_t SignedLeb128Size(int64_t data) {
  // Like UnsignedLeb128Size(), but we need one bit beyond the highest bit that differs from sign.
  uint64_t bits_to_encode = static_cast<uint64_t>(data ^ (data >> 63));
  uint32_t num_bits = 1 /* we need to encode the sign bit */ + 6 + 64 - CLZ(bits_to_encode | 1U);
  // See UnsignedLeb128Size for explanation. This is basically num_bits / 7.
  return (num_bits * 37) >> 8;
}

static inline uint8_t* EncodeUnsignedLeb128(uint8_t* dest, uint64_t value) {
  uint8_t out = value & 0x7f;
  value >>= 7;
  while (value != 0) {
    *dest++ = out | 0x80;
    out = value & 0x7f;
    value >>= 7;
  }
  *dest++ = out;
  return dest;
}

template <typename Vector>
static inline void EncodeUnsignedLeb128(Vector* dest, uint64_t value) {
  static_assert(std::is_same_v<typename Vector::value_type, uint8_t>, "Invalid value type");
  uint8_t out = value & 0x7f;
  value >>= 7;
  while (value != 0) {
    dest->push_back(out | 0x80);
    out = value & 0x7f;
    value >>= 7;
  }
  dest->push_back(out);
}

// Overwrite encoded Leb128 with a new value. The new value must be less than
// or equal to the old value to ensure that it fits the allocated space.
static inline void UpdateUnsignedLeb128(uint8_t* dest, uint32_t value) {
  const uint8_t* old_end = dest;
  uint32_t old_value = DecodeUnsignedLeb128(&old_end);
  DCHECK_LE(UnsignedLeb128Size(value), UnsignedLeb128Size(old_value));
  for (uint8_t* end = EncodeUnsignedLeb128(dest, value); end < old_end; end++) {
    // Use longer encoding than necessary to fill the allocated space.
    end[-1] |= 0x80;
    end[0] = 0;
  }
}

static inline uint8_t* EncodeSignedLeb128(uint8_t* dest, int64_t value) {
  uint64_t extra_bits = static_cast<uint64_t>(value ^ (value >> 63)) >> 6;
  uint8_t out = value & 0x7f;
  while (extra_bits != 0u) {
    *dest++ = out | 0x80;
    value >>= 7;
    out = value & 0x7f;
    extra_bits >>= 7;
  }
  *dest++ = out;
  return dest;
}

static inline void EncodeSignedLeb128(std::vector<uint8_t>* dest, int64_t value) {
  uint32_t extra_bits = static_cast<uint32_t>(value ^ (value >> 31)) >> 6;
  uint8_t out = value & 0x7f;
  while (extra_bits != 0u) {
    dest->push_back(out | 0x80);
    value >>= 7;
    out = value & 0x7f;
    extra_bits >>= 7;
  }
  dest->push_back(out);
}

// An encoder that pushes int32_t/uint32_t data onto the given std::vector.
template <typename Vector = std::vector<uint8_t>>
class Leb128Encoder {
  static_assert(std::is_same_v<typename Vector::value_type, uint8_t>, "Invalid value type");

 public:
  explicit Leb128Encoder(Vector* data) : data_(data) {
    DCHECK(data != nullptr);
  }

  void Reserve(uint32_t size) {
    data_->reserve(size);
  }

  void PushBackUnsigned(uint32_t value) {
    EncodeUnsignedLeb128(data_, value);
  }

  template<typename It>
  void InsertBackUnsigned(It cur, It end) {
    for (; cur != end; ++cur) {
      PushBackUnsigned(*cur);
    }
  }

  void PushBackSigned(int32_t value) {
    EncodeSignedLeb128(data_, value);
  }

  template<typename It>
  void InsertBackSigned(It cur, It end) {
    for (; cur != end; ++cur) {
      PushBackSigned(*cur);
    }
  }

  const Vector& GetData() const {
    return *data_;
  }

 protected:
  Vector* const data_;

 private:
  DISALLOW_COPY_AND_ASSIGN(Leb128Encoder);
};

// An encoder with an API similar to vector<uint32_t> where the data is captured in ULEB128 format.
template <typename Vector = std::vector<uint8_t>>
class Leb128EncodingVector final : private Vector,
                                   public Leb128Encoder<Vector> {
  static_assert(std::is_same_v<typename Vector::value_type, uint8_t>, "Invalid value type");

 public:
  Leb128EncodingVector() : Leb128Encoder<Vector>(this) { }

  explicit Leb128EncodingVector(const typename Vector::allocator_type& alloc)
    : Vector(alloc),
      Leb128Encoder<Vector>(this) { }

 private:
  DISALLOW_COPY_AND_ASSIGN(Leb128EncodingVector);
};

}  // namespace art

#endif  // ART_LIBARTBASE_BASE_LEB128_H_
