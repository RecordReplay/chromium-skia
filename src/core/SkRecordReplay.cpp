/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRecordReplay.h"

#include <dlfcn.h>

static void (*gRecordReplayAssert)(const char*, va_list);
static void (*gRecordReplayRegisterPointer)(const void* ptr);
static void (*gRecordReplayUnregisterPointer)(const void* ptr);
static int (*gRecordReplayPointerId)(const void* ptr);

static inline bool EnsureInitialized() {
  static bool initialized = false;
  if (!initialized) {
    RecordReplayLoadSymbol(handle, "RecordReplayAssert", gRecordReplayAssert);
    RecordReplayLoadSymbol(handle, "RecordReplayRegisterPointer", gRecordReplayRegisterPointer);
    RecordReplayLoadSymbol(handle, "RecordReplayUnregisterPointer", gRecordReplayUnregisterPointer);
    RecordReplayLoadSymbol(handle, "RecordReplayPointerId", gRecordReplayPointerId);
    initialized = true;
  }
  return !!gRecordReplayAssert;
}

void SkRecordReplayAssert(const char* format, ...) {
  if (EnsureInitialized()) {
    va_list ap;
    va_start(ap, format);
    gRecordReplayAssert(format, ap);
    va_end(ap);
  }
}

void SkRecordReplayRegisterPointer(const void* ptr) {
  if (EnsureInitialized()) {
    gRecordReplayRegisterPointer(ptr);
  }
}

void SkRecordReplayUnregisterPointer(const void* ptr) {
  if (EnsureInitialized()) {
    gRecordReplayUnregisterPointer(ptr);
  }
}

int SkRecordReplayPointerId(const void* ptr) {
  if (EnsureInitialized()) {
    return gRecordReplayPointerId(ptr);
  }
  return 0;
}
