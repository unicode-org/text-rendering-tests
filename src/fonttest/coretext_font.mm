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
#include <CoreText/CoreText.h>

#include "fonttest/coretext_font.h"
#include "fonttest/coretext_path.h"
#include "fonttest/font.h"

namespace fonttest {

CoreTextFont::CoreTextFont(CTFontDescriptorRef fontDescriptor)
  : fontDescriptor_(fontDescriptor) {
  CFRetain(fontDescriptor);
}

CoreTextFont::~CoreTextFont() {
  CFRelease(fontDescriptor_);
}

static CFNumberRef CreateAxisID(const std::string& tag) {
  uint8_t t[4];
  t[0] = tag.length() > 0 ? tag[0] : 0x20;
  t[1] = tag.length() > 1 ? tag[1] : 0x20;
  t[2] = tag.length() > 2 ? tag[2] : 0x20;
  t[3] = tag.length() > 3 ? tag[3] : 0x20;

  SInt32 value =
      (static_cast<SInt32>(t[0]) << 24) |
      (static_cast<SInt32>(t[1]) << 16) |
      (static_cast<SInt32>(t[2]) << 8) |
      (static_cast<SInt32>(t[3]) << 0);

  printf("***** ZEBRA %d\n", value);
  return CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
}

void CoreTextFont::GetGlyphOutline(int glyphID, const FontVariation& variation,
                                   std::string* path, std::string* viewBox) {
  CTFontDescriptorRef desc = fontDescriptor_;
  CFRetain(desc);
  for (FontVariation::const_iterator iter = variation.begin();
       iter != variation.end(); ++iter) {
    CFNumberRef axisID = CreateAxisID(iter->first);

    CTFontDescriptorRef oldDesc = desc;
    CGFloat axisValue = static_cast<CGFloat>(iter->second);
    printf("***** MOUSE %f\n", axisValue);
    desc = CTFontDescriptorCreateCopyWithVariation(desc, axisID, axisValue);
    CFRelease(oldDesc);
    CFRelease(axisID);
  }

  CTFontRef ctFont = CTFontCreateWithFontDescriptor(desc, 1000.0, NULL);
  for (FontVariation::const_iterator iter = variation.begin();
       iter != variation.end(); ++iter) {
    CFNumberRef axisID = CreateAxisID(iter->first);
    CFRelease(axisID);
  }

  CFDictionaryRef varDict = CTFontCopyVariation(ctFont);
  if (varDict) {
    NSString *s = [NSString stringWithFormat:@"%@", varDict];
    printf("****** %s\n", [s UTF8String]);
  }

  CGPathRef cgPath = NULL;
  if (ctFont) {
    cgPath =
        CTFontCreatePathForGlyph(ctFont, static_cast<CGGlyph>(glyphID), NULL);
  }
  if (cgPath) {
    *path = CoreTextPath(cgPath).ToSVGPath();
  }
  *viewBox = "TODO";

  if (cgPath) CFRelease(cgPath);
  if (ctFont) CFRelease(ctFont);
  CFRelease(desc);
}

}  // namespace fonttest
