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

static std::string GetGlyphName(CGFontRef font, CGGlyph glyph) {
  CFStringRef cfGlyphName = CGFontCopyGlyphNameForGlyph(font, glyph);
  char glyphName[512];
  if (!CFStringGetCString(cfGlyphName, glyphName, sizeof(glyphName),
                          kCFStringEncodingUTF8)) {
    snprintf(glyphName, sizeof(glyphName), "gid%u", glyph);
  }
  CFRelease(cfGlyphName);
  return std::string(glyphName);
}

bool CoreTextLine::RenderSVG(const std::string& idPrefix, std::string* svg) {
  CGFloat ascent, descent, leading;
  double width =
      CTLineGetTypographicBounds(line_, &ascent, &descent, &leading);

  std::string symbols, uses;
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

    CGFontRef cgFont = CTFontCopyGraphicsFont(font, NULL);
    CGAffineTransform transform = CTRunGetTextMatrix(run);
    CGGlyph glyphs[numGlyphs];
    CGPoint pos[numGlyphs];
    CFRange fullRange;
    fullRange.location = 0;
    fullRange.length = 0;  // length 0 means until end of run
    CTRunGetGlyphs(run, fullRange, glyphs);
    CTRunGetPositions(run, fullRange, pos);
    std::map<CGGlyph, std::string> glyphNames;
    for (CFIndex i = 0; i < numGlyphs; ++i) {
      if (glyphNames.find(glyphs[i]) != glyphNames.end()) {
        continue;
      }
      std::string glyphName = GetGlyphName(cgFont, glyphs[i]);
      glyphNames[glyphs[i]] = glyphName;
      symbols.append("  <symbol id=\"");
      symbols.append(idPrefix);
      symbols.append(".");
      symbols.append(glyphName);
      symbols.append("\" overflow=\"visible\"><path d=\"");
      CGPathRef path = CTFontCreatePathForGlyph(font, glyphs[i], NULL);
      symbols.append(CoreTextPath(path).ToSVGPath());
      CGPathRelease(path);
      symbols.append("\"/></symbol>\n");
    }

    for (CFIndex i = 0; i < numGlyphs; ++i) {
      char buffer[1024];
      snprintf(buffer, sizeof(buffer),
               "  <use xlink:href=\"#%s.%s\" x=\"%ld\" y=\"%ld\"/>\n",
               idPrefix.c_str(), glyphNames[glyphs[i]].c_str(),
               lround(pos[i].x), lround(pos[i].y));
      uses.append(buffer);
    }
    CGFontRelease(cgFont);
  }

  svg->clear();
  svg->append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	      "<svg version=\"1.1\"\n"
              "    xmlns=\"http://www.w3.org/2000/svg\"\n"
              "    xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
              "    viewBox=\"");
  char viewBox[200];
  snprintf(viewBox, sizeof(viewBox), "%ld %ld %ld %ld",
           0L, lround(-descent), lround(width), lround(ascent + descent));
  svg->append(viewBox);
  svg->append("\">\n");
  svg->append(symbols);
  svg->append(uses);
  svg->append("</svg>\n");
  return true;
}

}  // namespace fonttest
