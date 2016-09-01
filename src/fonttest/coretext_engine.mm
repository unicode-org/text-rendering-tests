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

#include <stdio.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#include <Foundation/Foundation.h>

#include "fonttest/font_engine.h"
#include "fonttest/coretext_engine.h"
#include "fonttest/coretext_font.h"
#include "fonttest/coretext_line.h"

namespace fonttest {

CoreTextEngine::CoreTextEngine() {
}

CoreTextEngine::~CoreTextEngine() {
}

std::string CoreTextEngine::GetName() const {
  return "CoreText";
}

Font* CoreTextEngine::LoadFont(const std::string& path, int faceIndex) {
  // Before MacOS 10.12, CoreText did not support variations on fonts
  // loaded from memory unless the CTFont was converted from a CGFont.
  // However, CGFonts can only be created from the first font in
  // a font container file. Therefore, we go via CoreGraphics
  // for faceIndex 0, and directly to CoreText for the others.
  if (faceIndex == 0) {
    CGDataProviderRef provider =
        CGDataProviderCreateWithFilename(path.c_str());
    if (!provider) {
      return NULL;
    }

    CGFontRef font = CGFontCreateWithDataProvider(provider);
    CGDataProviderRelease(provider);
    if (font == NULL) {
      return NULL;
    }

    return new CoreTextFont(font);
  }

  CFURLRef url = CFURLCreateFromFileSystemRepresentation(
      kCFAllocatorDefault,
      reinterpret_cast<const UInt8*>(path.c_str()), path.length(),
      /* not a directory */ false);
  CFArrayRef fonts = CTFontManagerCreateFontDescriptorsFromURL(url);
  if (!fonts) {
    return NULL;
  }

  CFIndex numFonts = CFArrayGetCount(fonts);
  if (faceIndex < 0 || faceIndex >= numFonts) {
    CFRelease(fonts);
    return NULL;
  }

  Font* result = NULL;
  CTFontDescriptorRef fontDesc = static_cast<CTFontDescriptorRef>(
      CFArrayGetValueAtIndex(fonts, faceIndex));
  if (fontDesc) {
    result = new CoreTextFont(fontDesc);
  }
  CFRelease(fonts);
  return result;
}

bool CoreTextEngine::RenderSVG(const std::string& text,
                               const std::string& textLanguage,
                               Font* font, double fontSize,
                               const FontVariation& fontVariation,
                               std::string* svg) {
  CoreTextFont* myFont = static_cast<CoreTextFont*>(font);
  CTFontRef ctFont = myFont->CreateFont(fontSize, fontVariation);
  CoreTextLine line(text, textLanguage,
                    myFont->CreateFont(fontSize, fontVariation));
  line.RenderSVG(svg);
  svg->clear();
  std::string path, viewBox;
  font->GetGlyphOutline(/* glyph id */ 1, fontVariation, &path, &viewBox);
  svg->append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	      "<svg viewBox=\"");
  svg->append(viewBox);
  svg->append("\"><g><path d=\"\n");
  svg->append(path);
  svg->append("\n\"></path></g></svg>\n");
  return true;
}

}  // namespace fonttest
