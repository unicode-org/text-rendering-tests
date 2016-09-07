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

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>

#include "raqm.h"
#include "fonttest/freestack_line.h"
#include "fonttest/freestack_path.h"

namespace fonttest {

FreeStackLine::FreeStackLine(
    const std::string& text, const std::string& textLanguage,
    FT_Face font, double fontSize)
  : line_(raqm_create()), font_(font), fontSize_(fontSize) {
  if (!line_ ||
      !raqm_set_text_utf8(line_, text.c_str(), text.length()) ||
      !raqm_set_language(line_, textLanguage.c_str(), 0, text.length()) ||
      !raqm_set_freetype_face(line_, font)) {
    std::cerr << "could not create Raqm line" << std::endl;
    exit(1);
  }
  if (!raqm_layout(line_)) {
    std::cerr << "raqm_layout() has failed" << std::endl;
    exit(1);
  }
}

FreeStackLine::~FreeStackLine() {
  raqm_destroy(line_);
}

bool FreeStackLine::RenderSVG(std::string* svg) {
  svg->clear();

  const double ascender = fontSize_ *
      (static_cast<double>(font_->ascender) /
       static_cast<double>(font_->units_per_EM));
  const double descender = fontSize_ *
      (static_cast<double>(font_->descender) /
       static_cast<double>(font_->units_per_EM));

  std::string path;
  double x = 0, y = 0;
  size_t numGlyphs = 0;
  raqm_glyph_t* glyphs = raqm_get_glyphs(line_, &numGlyphs);
  for (size_t i = 0; i < numGlyphs; ++i) {
    const raqm_glyph_t& glyph = glyphs[i];
    const double glyphX = x + glyph.x_offset;
    const double glyphY = y - glyph.y_offset;
    FT_Face font = glyph.ftface;
    FT_Error error =
        FT_Load_Glyph(font, glyph.index, FT_LOAD_NO_HINTING|FT_LOAD_NO_BITMAP);
    if (error) {
      std::cerr << "FT_Load_Glyph() failed; error: " << error << std::endl;
      exit(1);
    }

    if (!font->glyph) {
      std::cerr << "FT_Load_Glyph() did not load a glyph" << std::endl;
      exit(1);
    }

    FT_Vector transform;
    transform.x = static_cast<FT_Fixed>(glyphX);
    transform.y = static_cast<FT_Fixed>(glyphY);
    FreeTypePathConverter converter(transform);
    path.append(converter.Convert(&font->glyph->outline));

    x += glyphs[i].x_advance;
    y -= glyphs[i].y_advance;
  }

  //std::string path, viewBox;
  //font->GetGlyphOutline(/* glyph id */ 1, fontVariation, &path, &viewBox);

  svg->clear();
  svg->append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	      "<svg viewBox=\"");
  char viewBox[200];
  snprintf(viewBox, sizeof(viewBox), "%ld %ld %ld %ld",
           0L, lround(descender), lround(x), lround(ascender - descender));
  svg->append(viewBox);
  svg->append("\"><g><path d=\"\n");
  svg->append(path);
  svg->append("\"></path></g></svg>\n");
  return true;
}

}  // namespace fonttest
