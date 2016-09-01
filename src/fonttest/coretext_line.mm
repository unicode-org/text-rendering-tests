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

#include "fonttest/coretext_line.h"
#include "fonttest/coretext_path.h"

namespace fonttest {

static CFAttributedStringRef CreateAttrString(CFStringRef string,
                                              CTFontRef font) {
  CFStringRef keys[] = { kCTFontAttributeName };
  CFTypeRef values[] = { font };
  CFDictionaryRef attributes = CFDictionaryCreate(
      kCFAllocatorDefault, (const void**) &keys, (const void**) &values,
      sizeof(keys) / sizeof(keys[0]),
      &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFAttributedStringRef attrString =
      CFAttributedStringCreate(kCFAllocatorDefault, string, attributes);

  // TODO: We're seeing Lucida Grande here?!?!
  // printf("*** %s\n", [[attributes description] UTF8String]);
  CFRelease(attributes);
  return attrString;
}

CoreTextLine::CoreTextLine(const std::string& text,
                           const std::string& textLanguage,
                           CTFontRef font)
  : font_(font), line_(NULL) {
  CFRetain(font_);
  CFStringRef string = CFStringCreateWithCString(
      kCFAllocatorDefault, text.c_str(), kCFStringEncodingUTF8);
  CFAttributedStringRef attrString = CreateAttrString(string, font_);
  CFRelease(string);
  line_ = CTLineCreateWithAttributedString(attrString);
}

CoreTextLine::~CoreTextLine() {
  CFRelease(line_);
  CFRelease(font_);
}

bool CoreTextLine::RenderSVG(std::string* svg) {
  CFArrayRef glyphRuns = CTLineGetGlyphRuns(line_);
  CGFloat ascent, descent, leading;
  double width =
      CTLineGetTypographicBounds(line_, &ascent, &descent, &leading);
  // TODO: Why are we seeing Lucida Grande instead of our own font?!?!
  // printf("*** %s\n", [[glyphRuns description] UTF8String]);
  svg->clear();
  return true;
}

}  // namespace fonttest
