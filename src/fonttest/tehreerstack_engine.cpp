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

#include <iostream>
#include <sstream>

extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H
}

#include "fonttest/font_engine.h"
#include "fonttest/freestack_font.h"
#include "fonttest/tehreerstack_line.h"
#include "fonttest/tehreerstack_engine.h"

namespace fonttest {

TehreerStackEngine::TehreerStackEngine() {
  FT_Init_FreeType(&freeTypeLibrary_);
}

TehreerStackEngine::~TehreerStackEngine() {
  FT_Done_FreeType(freeTypeLibrary_);
}

std::string TehreerStackEngine::GetName() const {
  return "TehreerStack";
}

std::string TehreerStackEngine::GetVersion() const {
  std::stringstream result;

  FT_Int ftMajor, ftMinor, ftPatch;
  FT_Library_Version(freeTypeLibrary_, &ftMajor, &ftMinor, &ftPatch);
  result << "FreeType/" << ftMajor << '.' << ftMinor << '.' << ftPatch;

  result << " SheenBidi/2.0";

  result << " SheenFigure/1.5";

  return result.str();
}

Font* TehreerStackEngine::LoadFont(
    const std::string& path, int faceIndex) {
  FT_Face face = NULL;
  FT_Error error = FT_New_Face(freeTypeLibrary_, path.c_str(),
			       static_cast<FT_Long>(faceIndex), &face);
  if (error || !face) {
    return NULL;
  }

  return new FreeStackFont(face);
}

bool TehreerStackEngine::RenderSVG(const std::string& text,
                                   const std::string& textLanguage,
                                   Font* font, double fontSize,
                                   const FontVariation& fontVariation,
                                   const std::string& idPrefix,
                                   std::string* svg) {
  FT_Face face = static_cast<FreeStackFont*>(font)->GetFace(fontSize, fontVariation);
  TehreerStackLine line(text, textLanguage, face, fontSize);
  return line.RenderSVG(idPrefix, svg);
}

}  // namespace fonttest
