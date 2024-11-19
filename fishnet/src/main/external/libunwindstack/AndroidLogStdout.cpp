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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// This file contains only the log functions necessary to compile the unwinder
// tools using libdexfile for android targets.

extern "C" void __android_log_assert(const char* cond, const char*, const char* fmt, ...) {
  if (fmt) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
  } else {
    if (cond) {
      printf("Assertion failed: %s\n", cond);
    } else {
      printf("Unspecified assertion failed.\n");
    }
  }
  abort();
}
