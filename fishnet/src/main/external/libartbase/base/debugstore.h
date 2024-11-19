/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef ART_LIBARTBASE_BASE_DEBUGSTORE_H_
#define ART_LIBARTBASE_BASE_DEBUGSTORE_H_

#include <array>
#include <string>

#include "palette/palette.h"

namespace art {
static constexpr size_t STORE_MAX_SIZE = 1024;

inline std::string DebugStoreGetString() {
  std::array<char, STORE_MAX_SIZE> result{};
  // If PaletteDebugStoreGetString returns PALETTE_STATUS_NOT_SUPPORTED,
  // set an empty string as the result.
  result[0] = '\0';
  PaletteDebugStoreGetString(result.data(), result.size());
  return std::string(result.data());
}

}  // namespace art

#endif  // ART_LIBARTBASE_BASE_DEBUGSTORE_H_
