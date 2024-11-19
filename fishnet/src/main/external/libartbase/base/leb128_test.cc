/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include "leb128.h"

#include "gtest/gtest.h"
#include "histogram-inl.h"
#include "time_utils.h"

namespace art {

struct DecodeUnsignedLeb128TestCase {
  uint32_t decoded;
  uint8_t leb128_data[5];
};

static const DecodeUnsignedLeb128TestCase uleb128_tests[] = {
    {0,          {0, 0, 0, 0, 0}},
    {1,          {1, 0, 0, 0, 0}},
    {0x7F,       {0x7F, 0, 0, 0, 0}},
    {0x80,       {0x80, 1, 0, 0, 0}},
    {0x81,       {0x81, 1, 0, 0, 0}},
    {0xFF,       {0xFF, 1, 0, 0, 0}},
    {0x4000,     {0x80, 0x80, 1, 0, 0}},
    {0x4001,     {0x81, 0x80, 1, 0, 0}},
    {0x4081,     {0x81, 0x81, 1, 0, 0}},
    {0x0FFFFFFF, {0xFF, 0xFF, 0xFF, 0x7F, 0}},
    {0xFFFFFFFF, {0xFF, 0xFF, 0xFF, 0xFF, 0xF}},
};

struct Decode64bitUnsignedLeb128TestCase {
  uint64_t decoded;
  uint8_t leb128_data[10];
};

static const Decode64bitUnsignedLeb128TestCase uleb128_64bit_tests[] = {
    {0,                  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {1,                  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {0x7F,               {0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {0x80,               {0x80, 1, 0, 0, 0, 0, 0, 0, 0, 0}},
    {0x81,               {0x81, 1, 0, 0, 0, 0, 0, 0, 0, 0}},
    {0xFF,               {0xFF, 1, 0, 0, 0, 0, 0, 0, 0, 0}},
    {0x4000,             {0x80, 0x80, 1, 0, 0, 0, 0, 0, 0, 0}},
    {0x4001,             {0x81, 0x80, 1, 0, 0, 0, 0, 0, 0, 0}},
    {0x4081,             {0x81, 0x81, 1, 0, 0, 0, 0, 0, 0, 0}},
    {0x0FFFFFFF,         {0xFF, 0xFF, 0xFF, 0x7F, 0, 0, 0, 0, 0, 0}},
    {0xFFFFFFFF,         {0xFF, 0xFF, 0xFF, 0xFF, 0xF, 0, 0, 0, 0, 0}},
    {0x1FFFFFFFF,        {0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0, 0, 0, 0, 0}},
    {0x100000000,        {0x80, 0x80, 0x80, 0x80, 0x10, 0, 0, 0, 0, 0}},
    {0x8FFFFFFFF,        {0xFF, 0xFF, 0xFF, 0xFF, 0x8F, 0x01, 0, 0, 0, 0}},
    {0x8000000000000000, {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x01}},
    {0xFFFFFFFFFFFFFFFF, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01}},
};

struct DecodeSignedLeb128TestCase {
  int32_t decoded;
  uint8_t leb128_data[5];
};

static const DecodeSignedLeb128TestCase sleb128_tests[] = {
    {0,          {0, 0, 0, 0, 0}},
    {1,          {1, 0, 0, 0, 0}},
    {0x3F,       {0x3F, 0, 0, 0, 0}},
    {0x40,       {0xC0, 0 /* sign bit */, 0, 0, 0}},
    {0x41,       {0xC1, 0 /* sign bit */, 0, 0, 0}},
    {0x80,       {0x80, 1, 0, 0, 0}},
    {0xFF,       {0xFF, 1, 0, 0, 0}},
    {0x1FFF,     {0xFF, 0x3F, 0, 0, 0}},
    {0x2000,     {0x80, 0xC0, 0 /* sign bit */, 0, 0}},
    {0x2001,     {0x81, 0xC0, 0 /* sign bit */, 0, 0}},
    {0x2081,     {0x81, 0xC1, 0 /* sign bit */, 0, 0}},
    {0x4000,     {0x80, 0x80, 1, 0, 0}},
    {0x0FFFFF,   {0xFF, 0xFF, 0x3F, 0, 0}},
    {0x100000,   {0x80, 0x80, 0xC0, 0 /* sign bit */, 0}},
    {0x100001,   {0x81, 0x80, 0xC0, 0 /* sign bit */, 0}},
    {0x100081,   {0x81, 0x81, 0xC0, 0 /* sign bit */, 0}},
    {0x104081,   {0x81, 0x81, 0xC1, 0 /* sign bit */, 0}},
    {0x200000,   {0x80, 0x80, 0x80, 1, 0}},
    {0x7FFFFFF,  {0xFF, 0xFF, 0xFF, 0x3F, 0}},
    {0x8000000,  {0x80, 0x80, 0x80, 0xC0, 0 /* sign bit */}},
    {0x8000001,  {0x81, 0x80, 0x80, 0xC0, 0 /* sign bit */}},
    {0x8000081,  {0x81, 0x81, 0x80, 0xC0, 0 /* sign bit */}},
    {0x8004081,  {0x81, 0x81, 0x81, 0xC0, 0 /* sign bit */}},
    {0x8204081,  {0x81, 0x81, 0x81, 0xC1, 0 /* sign bit */}},
    {0x0FFFFFFF, {0xFF, 0xFF, 0xFF, 0xFF, 0 /* sign bit */}},
    {0x10000000, {0x80, 0x80, 0x80, 0x80, 1}},
    {0x7FFFFFFF, {0xFF, 0xFF, 0xFF, 0xFF, 0x7}},
    {-1,         {0x7F, 0, 0, 0, 0}},
    {-2,         {0x7E, 0, 0, 0, 0}},
    {-0x3F,      {0x41, 0, 0, 0, 0}},
    {-0x40,      {0x40, 0, 0, 0, 0}},
    {-0x41,      {0xBF, 0x7F, 0, 0, 0}},
    {-0x80,      {0x80, 0x7F, 0, 0, 0}},
    {-0x81,      {0xFF, 0x7E, 0, 0, 0}},
    {-0x00002000, {0x80, 0x40, 0, 0, 0}},
    {-0x00002001, {0xFF, 0xBF, 0x7F, 0, 0}},
    {-0x00100000, {0x80, 0x80, 0x40, 0, 0}},
    {-0x00100001, {0xFF, 0xFF, 0xBF, 0x7F, 0}},
    {-0x08000000, {0x80, 0x80, 0x80, 0x40, 0}},
    {-0x08000001, {0xFF, 0xFF, 0xFF, 0xBF, 0x7F}},
    {-0x20000000, {0x80, 0x80, 0x80, 0x80, 0x7E}},
    {static_cast<int32_t>(0x80000000), {0x80, 0x80, 0x80, 0x80, 0x78}},
};

struct Decode64bitSignedLeb128TestCase {
  int64_t decoded;
  uint8_t leb128_data[10];
};

static const Decode64bitSignedLeb128TestCase sleb128_64bit_tests[] = {
    {0,                   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {1,                   {1, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {0x3F,                {0x3F, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {0x40,                {0xC0, 0 /* sign bit */, 0, 0, 0, 0, 0, 0, 0, 0}},
    {0x800000000,         {0x80, 0x80, 0x80, 0x80, 0x80, 0x01, 0, 0, 0, 0}},
    {0x100000000,         {0x80, 0x80, 0x80, 0x80, 0x10, 0, 0, 0, 0, 0}},
    {0x700000000,         {0x80, 0x80, 0x80, 0x80, 0xF0, 0 /* sign bit */, 0, 0, 0, 0}},
    {0x704081002,         {0x82, 0xA0, 0xA0, 0xA0, 0xF0, 0 /* sign bit*/, 0, 0, 0}},
    {0x07FFFFFFFFFFFFFF,  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0}},
    {0x23FFFFFFFFFFFFFF,  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x23, 0}},
    {0x0800000000000000,  {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x08, 0}},
    {0x0800000008400421,  {0xA1, 0x88, 0x80, 0xC2, 0x80, 0x80, 0x80, 0x80, 0x08, 0}},
    {0x70FFFFFFFFFFFFFF,  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0 /* sign bit*/}},
    {0x7000000000000081,  {0x81, 0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xF0, 0 /* sign bit*/}},
    {0x0FFFFFFFFFFFFFFF,  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0}},
    {-1,                  {0x7F, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {-2,                  {0x7E, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {-0x3F,               {0x41, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {-0x40,               {0x40, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {-0x200000000,        {0x80, 0x80, 0x80, 0x80, 0x60, 0, 0, 0, 0}},
    {-0x200000001,        {0xFF, 0xFF, 0xFF, 0xFF, 0x5F, 0, 0, 0, 0}},
    {-0x0800000000000000, {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x78, 0}},
    {-0x0800000000000001, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0}},
    {-0x2000000000000000, {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x60, 0}},
    {static_cast<int64_t>(0x8000000000000000),
     {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F}},
};

TEST(Leb128Test, UnsignedSinglesVector) {
  // Test individual encodings.
  for (size_t i = 0; i < arraysize(uleb128_tests); ++i) {
    Leb128EncodingVector<> builder;
    builder.PushBackUnsigned(uleb128_tests[i].decoded);
    EXPECT_EQ(UnsignedLeb128Size(uleb128_tests[i].decoded), builder.GetData().size());
    const uint8_t* data_ptr = &uleb128_tests[i].leb128_data[0];
    const uint8_t* encoded_data_ptr = &builder.GetData()[0];
    for (size_t j = 0; j < 5; ++j) {
      if (j < builder.GetData().size()) {
        EXPECT_EQ(data_ptr[j], encoded_data_ptr[j]) << " i = " << i << " j = " << j;
      } else {
        EXPECT_EQ(data_ptr[j], 0U) << " i = " << i << " j = " << j;
      }
    }
    EXPECT_EQ(DecodeUnsignedLeb128(&data_ptr), uleb128_tests[i].decoded) << " i = " << i;
  }
}

TEST(Leb128Test, UnsignedSingles) {
  // Test individual encodings.
  for (size_t i = 0; i < arraysize(uleb128_tests); ++i) {
    uint8_t encoded_data[5];
    uint8_t* end = EncodeUnsignedLeb128(encoded_data, uleb128_tests[i].decoded);
    size_t data_size = static_cast<size_t>(end - encoded_data);
    EXPECT_EQ(UnsignedLeb128Size(uleb128_tests[i].decoded), data_size);
    const uint8_t* data_ptr = &uleb128_tests[i].leb128_data[0];
    for (size_t j = 0; j < 5; ++j) {
      if (j < data_size) {
        EXPECT_EQ(data_ptr[j], encoded_data[j]) << " i = " << i << " j = " << j;
      } else {
        EXPECT_EQ(data_ptr[j], 0U) << " i = " << i << " j = " << j;
      }
    }
    EXPECT_EQ(DecodeUnsignedLeb128(&data_ptr), uleb128_tests[i].decoded) << " i = " << i;
  }
}

TEST(Leb128Test, UnsignedSingles64bit) {
  // Test individual encodings.
  for (size_t i = 0; i < arraysize(uleb128_64bit_tests); ++i) {
    uint8_t encoded_data[10];
    uint8_t* end = EncodeUnsignedLeb128(encoded_data, uleb128_64bit_tests[i].decoded);
    size_t data_size = static_cast<size_t>(end - encoded_data);
    EXPECT_EQ(UnsignedLeb128Size(uleb128_64bit_tests[i].decoded), data_size);
    const uint8_t* data_ptr = &uleb128_64bit_tests[i].leb128_data[0];
    for (size_t j = 0; j < 10; ++j) {
      if (j < data_size) {
        EXPECT_EQ(data_ptr[j], encoded_data[j]) << " i = " << i << " j = " << j;
      } else {
        EXPECT_EQ(data_ptr[j], 0U) << " i = " << i << " j = " << j;
      }
    }
    EXPECT_EQ(DecodeUnsignedLeb128<uint64_t>(&data_ptr), uleb128_64bit_tests[i].decoded)
        << " i = " << i;
  }
}

TEST(Leb128Test, UnsignedStreamVector) {
  // Encode a number of entries.
  Leb128EncodingVector<> builder;
  for (size_t i = 0; i < arraysize(uleb128_tests); ++i) {
    builder.PushBackUnsigned(uleb128_tests[i].decoded);
  }
  const uint8_t* encoded_data_ptr = &builder.GetData()[0];
  for (size_t i = 0; i < arraysize(uleb128_tests); ++i) {
    const uint8_t* data_ptr = &uleb128_tests[i].leb128_data[0];
    for (size_t j = 0; j < UnsignedLeb128Size(uleb128_tests[i].decoded); ++j) {
      EXPECT_EQ(data_ptr[j], encoded_data_ptr[j]) << " i = " << i << " j = " << j;
    }
    for (size_t j = UnsignedLeb128Size(uleb128_tests[i].decoded); j < 5; ++j) {
      EXPECT_EQ(data_ptr[j], 0) << " i = " << i << " j = " << j;
    }
    EXPECT_EQ(DecodeUnsignedLeb128(&encoded_data_ptr), uleb128_tests[i].decoded) << " i = " << i;
  }
  EXPECT_EQ(builder.GetData().size(),
            static_cast<size_t>(encoded_data_ptr - &builder.GetData()[0]));
}

TEST(Leb128Test, UnsignedStream) {
  // Encode a number of entries.
  uint8_t encoded_data[5 * arraysize(uleb128_tests)];
  uint8_t* end = encoded_data;
  for (size_t i = 0; i < arraysize(uleb128_tests); ++i) {
    end = EncodeUnsignedLeb128(end, uleb128_tests[i].decoded);
  }
  size_t data_size = static_cast<size_t>(end - encoded_data);
  const uint8_t* encoded_data_ptr = encoded_data;
  for (size_t i = 0; i < arraysize(uleb128_tests); ++i) {
    const uint8_t* data_ptr = &uleb128_tests[i].leb128_data[0];
    for (size_t j = 0; j < UnsignedLeb128Size(uleb128_tests[i].decoded); ++j) {
      EXPECT_EQ(data_ptr[j], encoded_data_ptr[j]) << " i = " << i << " j = " << j;
    }
    for (size_t j = UnsignedLeb128Size(uleb128_tests[i].decoded); j < 5; ++j) {
      EXPECT_EQ(data_ptr[j], 0) << " i = " << i << " j = " << j;
    }
    EXPECT_EQ(DecodeUnsignedLeb128(&encoded_data_ptr), uleb128_tests[i].decoded) << " i = " << i;
  }
  EXPECT_EQ(data_size, static_cast<size_t>(encoded_data_ptr - encoded_data));
}

TEST(Leb128Test, Unsigned64bitStream) {
  // Encode a number of entries.
  uint8_t encoded_data[10 * arraysize(uleb128_64bit_tests)];
  uint8_t* end = encoded_data;
  for (size_t i = 0; i < arraysize(uleb128_64bit_tests); ++i) {
    end = EncodeUnsignedLeb128(end, uleb128_64bit_tests[i].decoded);
  }
  size_t data_size = static_cast<size_t>(end - encoded_data);
  const uint8_t* encoded_data_ptr = encoded_data;
  for (size_t i = 0; i < arraysize(uleb128_64bit_tests); ++i) {
    const uint8_t* data_ptr = &uleb128_64bit_tests[i].leb128_data[0];
    for (size_t j = 0; j < UnsignedLeb128Size(uleb128_64bit_tests[i].decoded); ++j) {
      EXPECT_EQ(data_ptr[j], encoded_data_ptr[j]) << " i = " << i << " j = " << j;
    }
    for (size_t j = UnsignedLeb128Size(uleb128_64bit_tests[i].decoded); j < 10; ++j) {
      EXPECT_EQ(data_ptr[j], 0U) << " i = " << i << " j = " << j;
    }
    EXPECT_EQ(DecodeUnsignedLeb128<uint64_t>(&encoded_data_ptr), uleb128_64bit_tests[i].decoded)
        << " i = " << i;
  }
  EXPECT_EQ(data_size, static_cast<size_t>(encoded_data_ptr - encoded_data));
}

TEST(Leb128Test, SignedSinglesVector) {
  // Test individual encodings.
  for (size_t i = 0; i < arraysize(sleb128_tests); ++i) {
    Leb128EncodingVector<> builder;
    builder.PushBackSigned(sleb128_tests[i].decoded);
    EXPECT_EQ(SignedLeb128Size(sleb128_tests[i].decoded), builder.GetData().size());
    const uint8_t* data_ptr = &sleb128_tests[i].leb128_data[0];
    const uint8_t* encoded_data_ptr = &builder.GetData()[0];
    for (size_t j = 0; j < 5; ++j) {
      if (j < builder.GetData().size()) {
        EXPECT_EQ(data_ptr[j], encoded_data_ptr[j]) << " i = " << i << " j = " << j;
      } else {
        EXPECT_EQ(data_ptr[j], 0U) << " i = " << i << " j = " << j;
      }
    }
    EXPECT_EQ(DecodeSignedLeb128(&data_ptr), sleb128_tests[i].decoded) << " i = " << i;
  }
}

TEST(Leb128Test, SignedSingles) {
  // Test individual encodings.
  for (size_t i = 0; i < arraysize(sleb128_tests); ++i) {
    uint8_t encoded_data[5];
    uint8_t* end = EncodeSignedLeb128(encoded_data, sleb128_tests[i].decoded);
    size_t data_size = static_cast<size_t>(end - encoded_data);
    EXPECT_EQ(SignedLeb128Size(sleb128_tests[i].decoded), data_size);
    const uint8_t* data_ptr = &sleb128_tests[i].leb128_data[0];
    for (size_t j = 0; j < 5; ++j) {
      if (j < data_size) {
        EXPECT_EQ(data_ptr[j], encoded_data[j]) << " i = " << i << " j = " << j;
      } else {
        EXPECT_EQ(data_ptr[j], 0U) << " i = " << i << " j = " << j;
      }
    }
    EXPECT_EQ(DecodeSignedLeb128(&data_ptr), sleb128_tests[i].decoded) << " i = " << i;
  }
}

TEST(Leb128Test, SignedSingles64bit) {
  // Test individual encodings.
  for (size_t i = 0; i < arraysize(sleb128_64bit_tests); ++i) {
    uint8_t encoded_data[10];
    uint8_t* end = EncodeSignedLeb128(encoded_data, sleb128_64bit_tests[i].decoded);
    size_t data_size = static_cast<size_t>(end - encoded_data);
    EXPECT_EQ(SignedLeb128Size(sleb128_64bit_tests[i].decoded), data_size);
    const uint8_t* data_ptr = &sleb128_64bit_tests[i].leb128_data[0];
    for (size_t j = 0; j < 10; ++j) {
      if (j < data_size) {
        EXPECT_EQ(data_ptr[j], encoded_data[j]) << " i = " << i << " j = " << j;
      } else {
        EXPECT_EQ(data_ptr[j], 0U) << " i = " << i << " j = " << j;
      }
    }
    EXPECT_EQ(DecodeSignedLeb128<int64_t>(&data_ptr), sleb128_64bit_tests[i].decoded)
        << " i = " << i;
  }
}

TEST(Leb128Test, SignedStreamVector) {
  // Encode a number of entries.
  Leb128EncodingVector<> builder;
  for (size_t i = 0; i < arraysize(sleb128_tests); ++i) {
    builder.PushBackSigned(sleb128_tests[i].decoded);
  }
  const uint8_t* encoded_data_ptr = &builder.GetData()[0];
  for (size_t i = 0; i < arraysize(sleb128_tests); ++i) {
    const uint8_t* data_ptr = &sleb128_tests[i].leb128_data[0];
    for (size_t j = 0; j < SignedLeb128Size(sleb128_tests[i].decoded); ++j) {
      EXPECT_EQ(data_ptr[j], encoded_data_ptr[j]) << " i = " << i << " j = " << j;
    }
    for (size_t j = SignedLeb128Size(sleb128_tests[i].decoded); j < 5; ++j) {
      EXPECT_EQ(data_ptr[j], 0) << " i = " << i << " j = " << j;
    }
    EXPECT_EQ(DecodeSignedLeb128(&encoded_data_ptr), sleb128_tests[i].decoded) << " i = " << i;
  }
  EXPECT_EQ(builder.GetData().size(),
            static_cast<size_t>(encoded_data_ptr - &builder.GetData()[0]));
}

TEST(Leb128Test, SignedStream) {
  // Encode a number of entries.
  uint8_t encoded_data[5 * arraysize(sleb128_tests)];
  uint8_t* end = encoded_data;
  for (size_t i = 0; i < arraysize(sleb128_tests); ++i) {
    end = EncodeSignedLeb128(end, sleb128_tests[i].decoded);
  }
  size_t data_size = static_cast<size_t>(end - encoded_data);
  const uint8_t* encoded_data_ptr = encoded_data;
  for (size_t i = 0; i < arraysize(sleb128_tests); ++i) {
    const uint8_t* data_ptr = &sleb128_tests[i].leb128_data[0];
    for (size_t j = 0; j < SignedLeb128Size(sleb128_tests[i].decoded); ++j) {
      EXPECT_EQ(data_ptr[j], encoded_data_ptr[j]) << " i = " << i << " j = " << j;
    }
    for (size_t j = SignedLeb128Size(sleb128_tests[i].decoded); j < 5; ++j) {
      EXPECT_EQ(data_ptr[j], 0) << " i = " << i << " j = " << j;
    }
    EXPECT_EQ(DecodeSignedLeb128(&encoded_data_ptr), sleb128_tests[i].decoded) << " i = " << i;
  }
  EXPECT_EQ(data_size, static_cast<size_t>(encoded_data_ptr - encoded_data));
}

TEST(Leb128Test, SignedStream64bit) {
  // Encode a number of entries.
  uint8_t encoded_data[10 * arraysize(sleb128_64bit_tests)];
  uint8_t* end = encoded_data;
  for (size_t i = 0; i < arraysize(sleb128_64bit_tests); ++i) {
    end = EncodeSignedLeb128(end, sleb128_64bit_tests[i].decoded);
  }
  size_t data_size = static_cast<size_t>(end - encoded_data);
  const uint8_t* encoded_data_ptr = encoded_data;
  for (size_t i = 0; i < arraysize(sleb128_64bit_tests); ++i) {
    const uint8_t* data_ptr = &sleb128_64bit_tests[i].leb128_data[0];
    for (size_t j = 0; j < SignedLeb128Size(sleb128_64bit_tests[i].decoded); ++j) {
      EXPECT_EQ(data_ptr[j], encoded_data_ptr[j]) << " i = " << i << " j = " << j;
    }
    for (size_t j = SignedLeb128Size(sleb128_64bit_tests[i].decoded); j < 10; ++j) {
      EXPECT_EQ(data_ptr[j], 0) << " i = " << i << " j = " << j;
    }
    EXPECT_EQ(DecodeSignedLeb128<int64_t>(&encoded_data_ptr), sleb128_64bit_tests[i].decoded)
        << " i = " << i;
  }
  EXPECT_EQ(data_size, static_cast<size_t>(encoded_data_ptr - encoded_data));
}

TEST(Leb128Test, UnsignedUpdate) {
  for (size_t i = 0; i < arraysize(uleb128_tests); ++i) {
    for (size_t j = 0; j < arraysize(uleb128_tests); ++j) {
      uint32_t old_value = uleb128_tests[i].decoded;
      uint32_t new_value = uleb128_tests[j].decoded;
      // We can only make the encoded value smaller.
      if (new_value <= old_value) {
        uint8_t encoded_data[5];
        uint8_t* old_end = EncodeUnsignedLeb128(encoded_data, old_value);
        UpdateUnsignedLeb128(encoded_data, new_value);
        const uint8_t* new_end = encoded_data;
        EXPECT_EQ(DecodeUnsignedLeb128(&new_end), new_value);
        // Even if the new value needs fewer bytes, we should fill the space.
        EXPECT_EQ(new_end, old_end);
      }
    }
  }
}

TEST(Leb128Test, Speed) {
  std::unique_ptr<Histogram<uint64_t>> enc_hist(new Histogram<uint64_t>("Leb128EncodeSpeedTest", 5));
  std::unique_ptr<Histogram<uint64_t>> dec_hist(new Histogram<uint64_t>("Leb128DecodeSpeedTest", 5));
  Leb128EncodingVector<> builder;
  // Push back 1024 chunks of 1024 values measuring encoding speed.
  uint64_t last_time = NanoTime();
  for (size_t i = 0; i < 1024; i++) {
    for (size_t j = 0; j < 1024; j++) {
      builder.PushBackUnsigned((i * 1024) + j);
    }
    uint64_t cur_time = NanoTime();
    enc_hist->AddValue(cur_time - last_time);
    last_time = cur_time;
  }
  // Verify encoding and measure decode speed.
  const uint8_t* encoded_data_ptr = &builder.GetData()[0];
  last_time = NanoTime();
  for (size_t i = 0; i < 1024; i++) {
    for (size_t j = 0; j < 1024; j++) {
      EXPECT_EQ(DecodeUnsignedLeb128(&encoded_data_ptr), (i * 1024) + j);
    }
    uint64_t cur_time = NanoTime();
    dec_hist->AddValue(cur_time - last_time);
    last_time = cur_time;
  }

  Histogram<uint64_t>::CumulativeData enc_data;
  enc_hist->CreateHistogram(&enc_data);
  enc_hist->PrintConfidenceIntervals(std::cout, 0.99, enc_data);

  Histogram<uint64_t>::CumulativeData dec_data;
  dec_hist->CreateHistogram(&dec_data);
  dec_hist->PrintConfidenceIntervals(std::cout, 0.99, dec_data);
}

}  // namespace art
