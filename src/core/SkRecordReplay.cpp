/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRecordReplay.h"

#ifndef _WIN32
#include <dlfcn.h>
#else
#include <windows.h>
#endif

#include <stdarg.h>
#include <string.h>

static bool (*gRecordReplayIsReplaying)();
static void (*gRecordReplayAssert)(const char*, va_list);
static void (*gRecordReplayRegisterPointer)(const void* ptr);
static void (*gRecordReplayUnregisterPointer)(const void* ptr);
static int (*gRecordReplayPointerId)(const void* ptr);
static bool (*gRecordReplayAreEventsDisallowed)();
static void (*gRecordReplayBeginPassThroughEvents)();
static void (*gRecordReplayEndPassThroughEvents)();

template <typename Src, typename Dst>
static inline void CastPointer(const Src src, Dst* dst) {
  static_assert(sizeof(Src) == sizeof(void*), "bad size");
  static_assert(sizeof(Dst) == sizeof(void*), "bad size");
  memcpy((void*)dst, (const void*)&src, sizeof(void*));
}

template <typename T>
static void RecordReplayLoadSymbol(const char* name, T& function) {
#ifndef _WIN32
  void* sym = dlsym(RTLD_DEFAULT, name);
#else
  HMODULE module = GetModuleHandleA("windows-recordreplay.dll");
  void* sym = module ? (void*)GetProcAddress(module, name) : nullptr;
#endif
  if (sym) {
    CastPointer(sym, &function);
  }
}

static inline bool EnsureInitialized() {
  static bool initialized = false;
  if (!initialized) {
    RecordReplayLoadSymbol("RecordReplayIsReplaying", gRecordReplayIsReplaying);
    RecordReplayLoadSymbol("RecordReplayAssert", gRecordReplayAssert);
    RecordReplayLoadSymbol("RecordReplayRegisterPointer", gRecordReplayRegisterPointer);
    RecordReplayLoadSymbol("RecordReplayUnregisterPointer", gRecordReplayUnregisterPointer);
    RecordReplayLoadSymbol("RecordReplayPointerId", gRecordReplayPointerId);
    RecordReplayLoadSymbol("RecordReplayAreEventsDisallowed", gRecordReplayAreEventsDisallowed);
    RecordReplayLoadSymbol("RecordReplayBeginPassThroughEvents", gRecordReplayBeginPassThroughEvents);
    RecordReplayLoadSymbol("RecordReplayEndPassThroughEvents", gRecordReplayEndPassThroughEvents);
    initialized = true;
  }
  return !!gRecordReplayAssert;
}

bool SkRecordReplayIsReplaying() {
  if (EnsureInitialized()) {
    return gRecordReplayIsReplaying();
  }
  return false;
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

bool SkRecordReplayAreEventsDisallowed() {
  if (EnsureInitialized()) {
    return gRecordReplayAreEventsDisallowed();
  }
  return false;
}

void SkRecordReplayBeginPassThroughEvents() {
  if (EnsureInitialized()) {
    gRecordReplayBeginPassThroughEvents();
  }
}

void SkRecordReplayEndPassThroughEvents() {
  if (EnsureInitialized()) {
    gRecordReplayEndPassThroughEvents();
  }
}
