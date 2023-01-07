// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=2a688e6f0a516c0d44a826381e9d637f
REG_FIDDLE(Bitmap_bytesPerPixel, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const char* colors[] = {"Unknown", "Alpha_8", "RGB_565", "ARGB_4444", "RGBA_8888", "RGB_888x",
                            "BGRA_8888", "RGBA_1010102", "RGB_101010x", "Gray_8", "RGBA_F16Norm",
                            "RGBA_F16"};
    SkImageInfo info = SkImageInfo::MakeA8(1, 1);
    SkBitmap bitmap;
    for (SkColorType colorType : {
    kUnknown_SkColorType, kAlpha_8_SkColorType, kRGB_565_SkColorType,
    kARGB_4444_SkColorType, kRGBA_8888_SkColorType, kRGB_888x_SkColorType,
    kBGRA_8888_SkColorType, kRGBA_1010102_SkColorType, kRGB_101010x_SkColorType,
    kGray_8_SkColorType, kRGBA_F16_SkColorType
                                 } ) {
        bitmap.setInfo(info.makeColorType(colorType));
        SkDebugf("color: k" "%s" "_SkColorType" "%*s" "bytesPerPixel: %d\n",
                colors[colorType], (int)(13 - strlen(colors[colorType])), " ",
                bitmap.bytesPerPixel());
    }
}
}  // END FIDDLE
