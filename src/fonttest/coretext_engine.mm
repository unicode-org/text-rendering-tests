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

#include <iostream>
#include <sstream>
#include <string>

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

std::string CoreTextEngine::GetVersion() const {
  std::stringstream result;

  result << "macOS/";
  NSProcessInfo *pInfo = [NSProcessInfo processInfo];
  result << pInfo.operatingSystemVersion.majorVersion
	 << '.' << pInfo.operatingSystemVersion.minorVersion
	 << '.' << pInfo.operatingSystemVersion.patchVersion
         << ' ';
  [pInfo release];

  const UInt32 ctVersion = CTGetCoreTextVersion();
  switch (ctVersion) {
  case kCTVersionNumber10_5:
    result << "CoreText/10.5";
    break;

  case kCTVersionNumber10_5_2:
    result << "CoreText/10.5.2";
    break;

  case kCTVersionNumber10_5_3:
    result << "CoreText/10.5.3";
    break;

  case kCTVersionNumber10_5_5:
    result << "CoreText/10.5.5";
    break;

  case kCTVersionNumber10_6:
    result << "CoreText/10.6";
    break;

  case kCTVersionNumber10_7:
    result << "CoreText/10.7";
    break;

  case kCTVersionNumber10_8:
    result << "CoreText/10.8";
    break;

  case kCTVersionNumber10_9:
    result << "CoreText/10.9";
    break;

  case kCTVersionNumber10_10:
    result << "CoreText/10.10";
    break;

  case kCTVersionNumber10_11:
    result << "CoreText/10.11";
    break;

  case kCTVersionNumber10_12:
    result << "CoreText/10.12";
    break;

  case kCTVersionNumber10_13:
    result << "CoreText/10.13";
    break;

#if defined(HAVE_CORETEXT_10_14) && HAVE_CORETEXT_10_14
  case kCTVersionNumber10_14:
    result << "CoreText/10.14";
    break;
#endif

  default:
    result << "CoreText/0x" << std::hex << ctVersion;
    break;
  }

  return result.str();
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
                               const std::string& idPrefix,
                               std::string* svg) {
  CoreTextFont* myFont = static_cast<CoreTextFont*>(font);
  CTFontRef ctFont = myFont->CreateFont(fontSize, fontVariation);
  CoreTextLine line(text, textLanguage, ctFont, fontSize);
  bool ok = line.RenderSVG(idPrefix, svg);
  CFRelease(ctFont);
  return ok;
}

}  // namespace fonttest
