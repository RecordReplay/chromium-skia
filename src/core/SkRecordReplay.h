/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecordReplay_DEFINED
#define SkRecordReplay_DEFINED

extern void SkRecordReplayPrint(const char* format, ...);
extern void SkRecordReplayWarning(const char* format, ...);
extern void SkRecordReplayAssert(const char* format, ...);
extern void SkRecordReplayRegisterPointer(const void* ptr);
extern void SkRecordReplayUnregisterPointer(const void* ptr);
extern int SkRecordReplayPointerId(const void* ptr);

extern bool SkRecordReplayAreEventsDisallowed();
extern void SkRecordReplayBeginPassThroughEvents();
extern void SkRecordReplayEndPassThroughEvents();
extern bool SkRecordReplayIsRecordingOrReplaying();
extern bool SkRecordReplayIsReplaying();
extern uintptr_t SkRecordReplayValue(const char* why, uintptr_t v);

#endif
