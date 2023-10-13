/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecordReplay_DEFINED
#define SkRecordReplay_DEFINED



// ##########################################################################
// V8 API
// ##########################################################################
#define ForEachV8API(Macro)                                             \
  Macro(IsRecordingOrReplaying,                                         \
        (const char* feature, const char* subfeature),                  \
        (feature, subfeature), bool, false)                             \
  Macro(IsRecording, (), (), bool, false)                               \
  Macro(IsReplaying, (), (), bool, false)                               \
  Macro(GetRecordingId, (), (), char*, nullptr)                         \
  Macro(RecordReplayValue,                                              \
        (const char* why, uintptr_t value), (why, value), uintptr_t, value) \
  Macro(RecordReplayCreateOrderedLock,                                  \
        (const char* name), (name), size_t, 0)                          \
  Macro(RecordReplayNewBookmark, (), (), uint64_t, 0)                   \
  Macro(RecordReplayAreEventsDisallowed,                                \
        (const char* why), (why), bool, false)                          \
  Macro(RecordReplayAreEventsPassedThrough,                             \
        (const char* why), (why), bool, false)                          \
  Macro(RecordReplayHasDivergedFromRecording, (), (), bool, false)      \
  Macro(RecordReplayAllowSideEffects, (), (), bool, true)               \
  Macro(RecordReplayPointerId, (const void* ptr), (ptr), int, 0)        \
  Macro(RecordReplayIdPointer, (int id), (id), void*, nullptr)          \
  Macro(RecordReplayFeatureEnabled,                                     \
        (const char* feature, const char* subfeature),                  \
        (feature, subfeature), bool, true)                              \
  Macro(RecordReplayHasDisabledFeatures, (), (), bool, false)           \
  Macro(RecordReplayAreAssertsDisabled, (), (), bool, false)            \
  Macro(IsMainThread, (), (), bool, false)                              \
  Macro(RecordReplayIsInReplayCode,                                     \
        (const char* why), (why), bool, false)                          \
  Macro(RecordReplayHadMismatch, (), (), bool, false  )

#define ForEachV8APIVoid(Macro)                                         \
  Macro(RecordReplayGetCurrentJSStack,                                  \
        (std::string* stackTrace), (stackTrace))

#define DefineFunction(Name, Formals, Args, ReturnType, DefaultValue) \
  ReturnType Sk##Name Formals;

  ForEachV8API(DefineFunction)
#undef DefineFunction

#define DefineFunctionVoid(Name, Formals, Args)                       \
  void Sk##Name Formals;

  ForEachV8APIVoid(DefineFunctionVoid)
#undef DefineFunction

extern void SkRecordReplayPrint(const char* format, ...);
extern void SkRecordReplayWarning(const char* format, ...);
extern void SkRecordReplayAssert(const char* format, ...);
extern void SkRecordReplayDiagnostic(const char* format, ...);
extern void SkRecordReplayRegisterPointer(const void* ptr);
extern void SkRecordReplayUnregisterPointer(const void* ptr);

extern bool SkRecordReplayIsRecordingOrReplaying(const char* feature = nullptr,
                                                 const char* subfeature = nullptr);
extern void SkRecordReplayBeginPassThroughEvents();
extern void SkRecordReplayEndPassThroughEvents();
extern bool SkRecordReplayIsReplaying();

#endif
