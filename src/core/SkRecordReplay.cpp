/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRecordReplay.h"

#include <dlfcn.h>
#include <stdarg.h>
#include <string.h>

static void (*gRecordReplayAssert)(const char*, va_list);
static void (*gRecordReplayRegisterPointer)(const void* ptr);
static void (*gRecordReplayUnregisterPointer)(const void* ptr);
static int (*gRecordReplayPointerId)(const void* ptr);

template <typename Src, typename Dst>
static inline void CastPointer(const Src src, Dst* dst) {
  static_assert(sizeof(Src) == sizeof(void*), "bad size");
  static_assert(sizeof(Dst) == sizeof(void*), "bad size");
  memcpy((void*)dst, (const void*)&src, sizeof(void*));
}

template <typename T>
static void RecordReplayLoadSymbol(const char* name, T& function) {
  void* sym = dlsym(RTLD_DEFAULT, name);
  if (sym) {
    CastPointer(sym, &function);
  }
}

static inline bool EnsureInitialized() {
  static bool initialized = false;
  if (!initialized) {
    RecordReplayLoadSymbol("RecordReplayAssert", gRecordReplayAssert);
    RecordReplayLoadSymbol("RecordReplayRegisterPointer", gRecordReplayRegisterPointer);
    RecordReplayLoadSymbol("RecordReplayUnregisterPointer", gRecordReplayUnregisterPointer);
    RecordReplayLoadSymbol("RecordReplayPointerId", gRecordReplayPointerId);
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
