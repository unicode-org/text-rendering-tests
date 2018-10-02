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

#include <ft2build.h>
#include FT_FREETYPE_H

#include "fonttest/font_engine.h"
#include "fonttest/freestack_engine.h"
#include "fonttest/freestack_font.h"
#include "fonttest/freestack_line.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb.h>

namespace fonttest {

FreeStackEngine::FreeStackEngine() {
  FT_Init_FreeType(&freeTypeLibrary_);
}

FreeStackEngine::~FreeStackEngine() {
  FT_Done_FreeType(freeTypeLibrary_);
}

std::string FreeStackEngine::GetName() const {
  return "FreeStack";
}

std::string FreeStackEngine::GetVersion() const {
  std::stringstream result;

  result << "HarfBuzz/" << hb_version_string() << ' ';

  FT_Int ftMajor, ftMinor, ftPatch;
  FT_Library_Version(freeTypeLibrary_, &ftMajor, &ftMinor, &ftPatch);
  result << "FreeType/" << ftMajor << '.' << ftMinor << '.' << ftPatch;

  return result.str();
}

Font* FreeStackEngine::LoadFont(
    const std::string& path, int faceIndex) {
  FT_Face face = NULL;
  FT_Error error = FT_New_Face(freeTypeLibrary_, path.c_str(),
			       static_cast<FT_Long>(faceIndex), &face);
  if (error || !face) {
    return NULL;
  }

  return new FreeStackFont(face);
}

bool FreeStackEngine::RenderSVG(const std::string& text,
                                const std::string& textLanguage,
                                Font* font, double fontSize,
                                const FontVariation& fontVariation,
                                const std::string& idPrefix,
                                std::string* svg) {
  FT_Face face =
      static_cast<FreeStackFont*>(font)->GetFace(fontSize, fontVariation);
  FreeStackLine line(text, textLanguage, face, fontSize);
  return line.RenderSVG(idPrefix, svg);
}

}  // namespace fonttest
