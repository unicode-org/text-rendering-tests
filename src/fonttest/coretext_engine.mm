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
#include <CoreText/CoreText.h>
#include <Foundation/Foundation.h>

#include "fonttest/font_engine.h"
#include "fonttest/coretext_engine.h"
#include "fonttest/coretext_font.h"

namespace fonttest {

CoreTextEngine::CoreTextEngine() {
}

CoreTextEngine::~CoreTextEngine() {
}

std::string CoreTextEngine::GetName() const {
  return "CoreText";
}

Font* CoreTextEngine::LoadFont(const std::string& path, int faceIndex) {
  NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
  NSURL* nsUrl = [NSURL fileURLWithPath:nsPath];
  CFURLRef url = static_cast<CFURLRef>(nsUrl);

  CFArrayRef fonts = CTFontManagerCreateFontDescriptorsFromURL(url);
  if (!fonts) {
    return NULL;
  }

  CFIndex numFonts = CFArrayGetCount(fonts);
  if (faceIndex < 0 || faceIndex >= numFonts) {
    CFRelease(fonts);
    return NULL;
  }

  Font* font = NULL;
  CTFontDescriptorRef fontDesc = static_cast<CTFontDescriptorRef>(
      CFArrayGetValueAtIndex(fonts, faceIndex));
  if (fontDesc) {
    font = new CoreTextFont(fontDesc);
  }
  CFRelease(fonts);
  return font;
}

}  // namespace fonttest
