/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkOnce.h"
#include "SkRecordReplay.h"

#ifndef _WIN32
#include <dlfcn.h>
#else
#include <windows.h>
#endif

#include <stdarg.h>
#include <string.h>

#include <string>

static inline bool EnsureInitialized();


#if defined(_WIN32)

#define DefineFunction(Name, Formals, Args, ReturnType, DefaultValue) \
  static ReturnType (*gV8##Name) Formals;                             \
  static inline ReturnType V8##Name Formals {                         \
    EnsureInitialized();                                              \
    return gV8##Name ? gV8##Name Args : DefaultValue;                 \
  }
        ForEachV8API(DefineFunction)
#undef DefineFunction

#define DefineFunctionVoid(Name, Formals, Args) \
  static void (*gV8##Name) Formals;             \
  static inline void V8##Name Formals {         \
    EnsureInitialized();                        \
    if (gV8##Name)                              \
      gV8##Name Args;                           \
  }
                ForEachV8APIVoid(DefineFunctionVoid)
#undef DefineFunctionVoid

static void InitializationError(const char* format, ...) {
  {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
  }

#if defined(_WIN32)
  // Additionally write the message to a new file. Capturing the output written to
  // stderr by browser subprocesses on windows is surprisingly difficult.
  const char* dir = getenv("RECORD_REPLAY_LOG_DIRECTORY");
  char file[1024];
  snprintf(file, sizeof(file), "%s\\record_replay_initialization_error.txt", dir ? dir : ".");
  FILE* f = fopen(file, "w");
  if (f) {
    va_list args;
    va_start(args, format);
    vfprintf(f, format, args);
    va_end(args);
    fclose(f);
  }
#endif

  CHECK(0);
}

static void InitV8Bindings() {
  HMODULE module;
  if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
			  reinterpret_cast<LPCSTR>(InitV8Bindings),
			  &module)) {
    InitializationError("GetModuleHandleExA failed %d, crashing...\n", (int)GetLastError());
  }

#define LoadFunction(Name, Formals, Args, ReturnType, DefaultValue)           \
  gV8##Name = reinterpret_cast<ReturnType(*)Formals>(GetProcAddress(module, #Name))\
  if (!gV8##Name) {                                                           \
    InitializationError("Could not find V8 export %s, crashing...\n", #Name); \
  }
ForEachV8API(LoadFunction)
#undef LoadFunction

#define LoadFunctionVoid(Name, Formals, Args)                                 \
  gV8##Name = reinterpret_cast<void(*)Formals>(GetProcAddress(module, #Name))\
  if (!gV8##Name) {                                                           \
    InitializationError("Could not find V8 export %s, crashing...\n", #Name); \
  }
ForEachV8APIVoid(LoadFunctionVoid)
#undef LoadFunctionVoid
}

#else // !BUILD_FLAG(IS_WIN)

#define DefineFunction(Name, Formals, Args, ReturnType, DefaultValue) \
  extern "C" ReturnType V8##Name Formals;                             \
  ReturnType Sk##Name Formals {                                       \
    return V8##Name Args;                                             \
  }
        ForEachV8API(DefineFunction)
#undef DefineFunction

#define DefineFunctionVoid(Name, Formals, Args)                       \
  extern "C" void V8##Name Formals;                                   \
  void Sk##Name Formals {                                             \
    V8##Name Args;                                                    \
  }
                ForEachV8APIVoid(DefineFunctionVoid)
#undef DefineFunction

static void InitV8Bindings() {
}

#endif // !BUILD_FLAG(IS_WIN)

// ##########################################################################
// Driver/Linker API
// ##########################################################################
static bool gRecordingOrReplaying;
static bool gHasDisabledFeatures;

static void (*gRecordReplayPrint)(const char* format, va_list args);
static void (*gRecordReplayWarning)(const char* format, va_list args);
static void (*gRecordReplayAssert)(const char*, va_list);
static void (*gRecordReplayDiagnostic)(const char*, va_list);
static void (*gRecordReplayRegisterPointer)(const void* ptr);
static void (*gRecordReplayUnregisterPointer)(const void* ptr);
static int (*gRecordReplayPointerId)(const void* ptr);

static bool (*gRecordReplayHasDisabledFeatures)();
static bool (*gRecordReplayFeatureEnabled)(const char* feature, const char* subfeature);
static bool (*gRecordReplayAreEventsDisallowed)();
static void (*gRecordReplayBeginPassThroughEvents)();
static void (*gRecordReplayEndPassThroughEvents)();
static bool (*gRecordReplayIsReplaying)(void);
static uintptr_t (*gRecordReplayValue)(const char* why, uintptr_t v);

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
  static SkOnce once;
  once([]{
    InitV8Bindings();

    RecordReplayLoadSymbol("RecordReplayPrint", gRecordReplayPrint);
    RecordReplayLoadSymbol("RecordReplayWarning", gRecordReplayWarning);
    RecordReplayLoadSymbol("RecordReplayAssert", gRecordReplayAssert);
    RecordReplayLoadSymbol("RecordReplayDiagnostic", gRecordReplayDiagnostic);
    RecordReplayLoadSymbol("RecordReplayRegisterPointer", gRecordReplayRegisterPointer);
    RecordReplayLoadSymbol("RecordReplayUnregisterPointer", gRecordReplayUnregisterPointer);
    RecordReplayLoadSymbol("RecordReplayPointerId", gRecordReplayPointerId);

    RecordReplayLoadSymbol("RecordReplayHasDisabledFeatures", gRecordReplayHasDisabledFeatures);
    RecordReplayLoadSymbol("RecordReplayFeatureEnabled", gRecordReplayFeatureEnabled);
    RecordReplayLoadSymbol("RecordReplayAreEventsDisallowed", gRecordReplayAreEventsDisallowed);
    RecordReplayLoadSymbol("RecordReplayBeginPassThroughEvents", gRecordReplayBeginPassThroughEvents);
    RecordReplayLoadSymbol("RecordReplayEndPassThroughEvents", gRecordReplayEndPassThroughEvents);
    RecordReplayLoadSymbol("RecordReplayIsReplaying", gRecordReplayIsReplaying);

    gRecordingOrReplaying =
            gRecordReplayFeatureEnabled && gRecordReplayFeatureEnabled("record-replay", nullptr);
    gHasDisabledFeatures = gRecordingOrReplaying && gRecordReplayHasDisabledFeatures();
  });
  return !!gRecordReplayAssert;
}

void SkRecordReplayPrint(const char* format, ...) {
  if (SkRecordReplayIsRecordingOrReplaying()) {
    va_list args;
    va_start(args, format);
    gRecordReplayPrint(format, args);
    va_end(args);
  }
}

void SkRecordReplayWarning(const char* format, ...) {
  if (EnsureInitialized()) {
    va_list ap;
    va_start(ap, format);
    gRecordReplayWarning(format, ap);
    va_end(ap);
  }
}

void SkRecordReplayAssert(const char* format, ...) {
  if (EnsureInitialized()) {
    va_list ap;
    va_start(ap, format);
    gRecordReplayAssert(format, ap);
    va_end(ap);
  }
}

void SkRecordReplayDiagnostic(const char* format, ...) {
  if (EnsureInitialized()) {
    va_list ap;
    va_start(ap, format);
    gRecordReplayDiagnostic(format, ap);
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

bool SkRecordReplayIsRecordingOrReplaying(const char* feature, const char* subfeature) {
  return EnsureInitialized() && gRecordingOrReplaying &&
    (!feature || SkRecordReplayFeatureEnabled(feature, subfeature));
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

bool SkRecordReplayIsReplaying(void) {
  return EnsureInitialized() && gRecordReplayIsReplaying();
}
