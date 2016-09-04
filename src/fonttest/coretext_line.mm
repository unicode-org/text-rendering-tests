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
  CFRelease(attributes);
  return attrString;
}

static bool HasTable(CTFontRef font, CTFontTableTag tableTag) {
  bool result = false;
  CFArrayRef tables =
      CTFontCopyAvailableTables(font, kCTFontTableOptionNoOptions);
  CFIndex numTables =  CFArrayGetCount(tables);
  for (CFIndex i = 0; i < numTables; ++i) {
    if (reinterpret_cast<uintptr_t>(CFArrayGetValueAtIndex(tables, i))
        == static_cast<uintptr_t>(tableTag)) {
      result = true;
      break;
    }
  }
  CFRelease(tables);
  return result;
}

CoreTextLine::CoreTextLine(const std::string& text,
                           const std::string& textLanguage,
                           CTFontRef font, double fontSize)
  : font_(font), line_(NULL), fontSize_(fontSize) {
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
  std::string svgPath;
  CGFloat ascent, descent, leading;
  double width =
      CTLineGetTypographicBounds(line_, &ascent, &descent, &leading);

  // CoreText seems to have a off-by-one error when converting typographic
  // descents, but strangely it only happens with TrueType fonts.
  // The problem has been reported to Apple. We’re working around it here
  // because it would make very many test cases fail, which might arguably
  // be the correct thing to do, but it would possibly hide other errors
  // which can have a larger impact than a descent value that is of by 0.1%.
  if (HasTable(font_, kCTFontTableGlyf)) {
    descent = descent + (fontSize_ / CTFontGetUnitsPerEm(font_));
  }

  CFArrayRef runs = CTLineGetGlyphRuns(line_);
  CFIndex numRuns = CFArrayGetCount(runs);
  for (CFIndex runIndex = 0; runIndex < numRuns; ++runIndex) {
    CTRunRef run =
      static_cast<CTRunRef>(CFArrayGetValueAtIndex(runs, runIndex));
    CFIndex numGlyphs = CTRunGetGlyphCount(run);
    CFDictionaryRef attrs = CTRunGetAttributes(run);
    if (numGlyphs == 0 || !attrs) {
      continue;
    }

    CTFontRef font = static_cast<CTFontRef>(
        CFDictionaryGetValue(attrs, kCTFontAttributeName));
    if (!font) {
      continue;
    }

    CGAffineTransform transform = CTRunGetTextMatrix(run);
    CGGlyph glyphs[numGlyphs];
    CGPoint pos[numGlyphs];
    CFRange fullRange;
    fullRange.location = 0;
    fullRange.length = 0;  // length 0 means until end of run
    CTRunGetGlyphs(run, fullRange, glyphs);
    CTRunGetPositions(run, fullRange, pos);
    for (CFIndex i = 0; i < numGlyphs; ++i) {
      transform.tx = pos[i].x;
      transform.ty = pos[i].y;
      CGPathRef path =
          CTFontCreatePathForGlyph(font, glyphs[i], &transform);
      if (path) {
        svgPath.append(CoreTextPath(path).ToSVGPath());
        CGPathRelease(path);
      }
    }
  }

  svg->clear();
  svg->append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	      "<svg viewBox=\"");
  char viewBox[200];
  snprintf(viewBox, sizeof(viewBox), "%ld %ld %ld %ld",
           static_cast<long>(0), static_cast<long>(-descent),
           static_cast<long>(width), static_cast<long>(ascent + descent));
  svg->append(viewBox);
  svg->append("\"><g><path d=\"\n");
  svg->append(svgPath);
  svg->append("\n\"></path></g></svg>\n");
  return true;
}

}  // namespace fonttest
