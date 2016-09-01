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

#include "fonttest/font_engine.h"
#include "fonttest/fthb_engine.h"
#include "fonttest/fthb_font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace fonttest {

FreeTypeHarfBuzzEngine::FreeTypeHarfBuzzEngine() {
  FT_Init_FreeType(&freeTypeLibrary_);
}

FreeTypeHarfBuzzEngine::~FreeTypeHarfBuzzEngine() {
  FT_Done_FreeType(freeTypeLibrary_);
}

std::string FreeTypeHarfBuzzEngine::GetName() const {
  return "FreeType/HarfBuzz";
}

Font* FreeTypeHarfBuzzEngine::LoadFont(
    const std::string& path, int faceIndex) {
  FT_Face face = NULL;
  FT_Error error = FT_New_Face(freeTypeLibrary_, path.c_str(),
			       static_cast<FT_Long>(faceIndex), &face);
  if (error || !face) {
    return NULL;
  }

  return new FreeTypeHarfBuzzFont(face);
}

bool FreeTypeHarfBuzzEngine::RenderSVG(const std::string& text,
                                       const std::string& textLanguage,
                                       Font* font, double fontSize,
                                       const FontVariation& fontVariation,
                                       std::string* svg) {
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
