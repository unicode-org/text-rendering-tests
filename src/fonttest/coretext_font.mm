/* Copyright 2016 Unicode Inc. All rights reserved.
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

#include <cstdint>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>

#include "fonttest/coretext_font.h"
#include "fonttest/coretext_path.h"
#include "fonttest/font.h"

namespace fonttest {

CoreTextFont::CoreTextFont(CTFontDescriptorRef fontDescriptor)
  : fontDescriptor_(fontDescriptor), cgFont_(NULL) {
  CFRetain(fontDescriptor);
}

CoreTextFont::CoreTextFont(CGFontRef cgFont)
  : fontDescriptor_(NULL), cgFont_(cgFont) {
  CGFontRetain(cgFont);
}

CoreTextFont::~CoreTextFont() {
  if (fontDescriptor_) {
    CFRelease(fontDescriptor_);
  }
  if (cgFont_) {
    CGFontRelease(cgFont_);
  }
}

static SInt32 GetAxisID(const std::string& tag) {
  uint8_t t[4];
  t[0] = tag.length() > 0 ? tag[0] : 0x20;
  t[1] = tag.length() > 1 ? tag[1] : 0x20;
  t[2] = tag.length() > 2 ? tag[2] : 0x20;
  t[3] = tag.length() > 3 ? tag[3] : 0x20;

  return
      (static_cast<SInt32>(t[0]) << 24) |
      (static_cast<SInt32>(t[1]) << 16) |
      (static_cast<SInt32>(t[2]) << 8) |
      (static_cast<SInt32>(t[3]) << 0);
}

static CFNumberRef CreateAxisID(const std::string& tag) {
  SInt32 value = GetAxisID(tag);
  return CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
}

static void SetVariationDictionary(const FontVariation& fvar,
                                   CFMutableDictionaryRef dict) {
  if (fvar.empty()) {
    return;
  }

  const void* keys[fvar.size()];
  const void* values[fvar.size()];
  size_t i = 0;
  for (FontVariation::const_iterator iter = fvar.begin();
       iter != fvar.end(); ++iter, ++i) {
    keys[i] = CreateAxisID(iter->first);
    values[i] = CFNumberCreate(kCFAllocatorDefault,
                               kCFNumberDoubleType, &iter->second);
  }

  CFDictionaryRef vardict = CFDictionaryCreate(
      kCFAllocatorDefault, keys, values, fvar.size(),
      &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFDictionarySetValue(dict, CFSTR("NSCTFontVariationAttribute"), vardict);
  CFRelease(vardict);
}

static CGFontRef CreateVariationCGFont(CGFontRef font,
                                       const FontVariation& variation) {
  if (variation.empty()) {
    CGFontRetain(font);
    return font;
  }

  // Before MacOSX 10.12, we need to specify variations by axis name
  // because variations did not work for CoreText fonts loaded at runtime.
  // To build a mapping from axis tag to name, we can query CoreText.
  std::map<SInt32, CFStringRef> axisNames;
  {
    CTFontRef ctFont = CTFontCreateWithGraphicsFont(font, 12.0, NULL, NULL);
    CFArrayRef axes = CTFontCopyVariationAxes(ctFont);
    CFIndex numAxes = CFArrayGetCount(axes);
    for (CFIndex axisIndex = 0; axisIndex < numAxes; ++axisIndex) {
      CFDictionaryRef axis = static_cast<CFDictionaryRef>(
          CFArrayGetValueAtIndex(axes, axisIndex));
      CFNumberRef axisID = static_cast<CFNumberRef>(
          CFDictionaryGetValue(axis, kCTFontVariationAxisIdentifierKey));
      SInt32 axisTag;
      if (CFNumberGetValue(axisID, kCFNumberSInt32Type, &axisTag)) {
        CFStringRef axisName = static_cast<CFStringRef>(
          CFDictionaryGetValue(axis, kCTFontVariationAxisNameKey));
        CFRetain(axisName);
        axisNames[axisTag] = axisName;
      }
    }
    CFRelease(axes);
    CFRelease(ctFont);
  }

  const void* keys[variation.size()];
  const void* values[variation.size()];
  size_t numAxesUsed = 0;
  for (FontVariation::const_iterator iter = variation.begin();
       iter != variation.end(); ++iter) {
    auto axisNameIter = axisNames.find(GetAxisID(iter->first));
    if (axisNameIter != axisNames.end()) {
      keys[numAxesUsed] = axisNameIter->second;
      values[numAxesUsed] = CFNumberCreate(kCFAllocatorDefault,
                                           kCFNumberDoubleType, &iter->second);
      ++numAxesUsed;
    }
  }

  CFDictionaryRef vardict = CFDictionaryCreate(
      kCFAllocatorDefault, keys, values, numAxesUsed,
      &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

  for (auto iter = axisNames.begin(); iter != axisNames.end(); ++iter) {
    CFRelease(iter->second);
  }

  return CGFontCreateCopyWithVariations(font, vardict);
}

static CTFontDescriptorRef CreateVariationFontDescriptor(
      CTFontDescriptorRef desc,
      const FontVariation& variation) {
  CFRetain(desc);
  for (FontVariation::const_iterator iter = variation.begin();
       iter != variation.end(); ++iter) {
    CFNumberRef axisID = CreateAxisID(iter->first);
    CTFontDescriptorRef oldDesc = desc;
    CGFloat axisValue = static_cast<CGFloat>(iter->second);
    desc = CTFontDescriptorCreateCopyWithVariation(desc, axisID, axisValue);
    CFRelease(oldDesc);
    CFRelease(axisID);
  }
  return desc;
}

CTFontRef CoreTextFont::CreateFont(double size,
                                   const FontVariation& variation) {
  if (cgFont_) {
    CGFontRef varCGFont = CreateVariationCGFont(cgFont_, variation);
    return CTFontCreateWithGraphicsFont(varCGFont, size, NULL, NULL);
  }

  CTFontDescriptorRef desc =
      CreateVariationFontDescriptor(fontDescriptor_, variation);
  CTFontRef font = CTFontCreateWithFontDescriptor(desc, size, NULL);
  CFRelease(desc);

  return font;
}

void CoreTextFont::GetGlyphOutline(int glyphID, const FontVariation& variation,
                                   std::string* path, std::string* viewBox) {
  CTFontRef font = CreateFont(1000.0, variation);
  CGFloat ascent = CTFontGetAscent(font);
  CGFloat descent = CTFontGetDescent(font) + 1;  // TODO

  CGGlyph glyphs[1];
  glyphs[0] = static_cast<CGGlyph>(glyphID);
  CGSize advances[1];
  CTFontGetAdvancesForGlyphs(font, kCTFontHorizontalOrientation, glyphs,
                             advances, 1);
  char buffer[200];
  snprintf(buffer, sizeof(buffer), "%ld %ld %ld %ld",
           static_cast<long>(0),
           static_cast<long>(-descent),
           static_cast<long>(advances[0].width),
           static_cast<long>(ascent + descent));
  viewBox->assign(buffer);

  CGPathRef cgPath = NULL;
  if (font) {
    cgPath =
        CTFontCreatePathForGlyph(font, static_cast<CGGlyph>(glyphID), NULL);
  }
  if (cgPath) {
    *path = CoreTextPath(cgPath).ToSVGPath();
  }

  if (cgPath) CGPathRelease(cgPath);
  CFRelease(font);
}

}  // namespace fonttest
