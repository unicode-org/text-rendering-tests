/* Copyright 2019 Unicode Inc. All rights reserved.
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

#ifndef FONTTEST_TEHREERSTACK_LINE_H_
#define FONTTEST_TEHREERSTACK_LINE_H_

#include <cstdint>
#include <string>
#include <vector>

extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H

#include <SheenFigure.h>
}

#include "fonttest/font.h"

struct GlyphInfo {
    uint16_t glyphID;
    double xOffset;
    double yOffset;
    double advance;
};

namespace fonttest {

class TehreerStackLine {
 public:
  TehreerStackLine(const std::string& text, const std::string& textLanguage,
                   FT_Face font, double fontSize);
  ~TehreerStackLine();
  bool RenderSVG(const std::string& idPrefix, std::string* svg);

 private:
  SFFontRef sfFont_;
  FT_Face font_;
  double fontSize_;

  std::vector<GlyphInfo> glyphInfos;
};

}  // namespace fonttest

#endif  // FONTTEST_TEHREERSTACK_LINE_H_
