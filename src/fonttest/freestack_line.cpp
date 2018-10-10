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
#include <set>
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
      !raqm_set_invisible_glyph(line_, -1) ||
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

bool FreeStackLine::RenderSVG(const std::string& idPrefix, std::string* svg) {
  svg->clear();

  const double ascender = fontSize_ *
      (static_cast<double>(font_->ascender) /
       static_cast<double>(font_->units_per_EM));
  const double descender = fontSize_ *
      (static_cast<double>(font_->descender) /
       static_cast<double>(font_->units_per_EM));

  size_t numGlyphs = 0;
  raqm_glyph_t* glyphs = raqm_get_glyphs(line_, &numGlyphs);

  std::string symbols;
  std::map<int, std::string> glyphNames;
  for (size_t i = 0; i < numGlyphs; ++i) {
    const raqm_glyph_t& glyph = glyphs[i];
    if (glyphNames.find(glyph.index) != glyphNames.end()) {
      continue;
    }

    FT_Face font = glyph.ftface;
    char glyphName[512];
    FT_Error error =
        FT_Get_Glyph_Name(font, glyph.index, glyphName, sizeof(glyphName));
    if (error || *glyphName == '\0') {
      snprintf(glyphName, sizeof(glyphName), "gid%u", glyph.index);
    }
    glyphNames[glyph.index] = std::string(glyphName);

    error =
        FT_Load_Glyph(font, glyph.index, FT_LOAD_NO_HINTING|FT_LOAD_NO_BITMAP);
    if (error) {
      std::cerr << "FT_Load_Glyph() failed; error: " << error << std::endl;
      exit(1);
    }

    if (!font->glyph) {
      std::cerr << "FT_Load_Glyph() did not load a glyph" << std::endl;
      exit(1);
    }

    symbols.append("  <symbol id=\"");
    symbols.append(idPrefix);
    symbols.append(".");
    symbols.append(glyphName);
    symbols.append("\" overflow=\"visible\"><path d=\"");
    FT_Vector transform;
    transform.x = transform.y = 0;
    FreeTypePathConverter converter(transform);
    symbols.append(converter.Convert(&font->glyph->outline));
    symbols.append("\"/></symbol>\n");
  }

  std::string uses;
  double x = 0, y = 0;
  for (size_t i = 0; i < numGlyphs; ++i) {
    const raqm_glyph_t& glyph = glyphs[i];
    const double glyphX = x + glyph.x_offset;
    const double glyphY = y + glyph.y_offset;
    char buffer[1024];
    snprintf(buffer, sizeof(buffer),
             "  <use xlink:href=\"#%s.%s\" x=\"%ld\" y=\"%ld\"/>\n",
             idPrefix.c_str(), glyphNames[glyph.index].c_str(),
             lround(glyphX / 64), lround(glyphY / 64));
    uses.append(buffer);
    x += glyph.x_advance;
    y += glyph.y_advance;
  }

  svg->clear();
  svg->append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<svg version=\"1.1\"\n"
              "    xmlns=\"http://www.w3.org/2000/svg\"\n"
              "    xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
              "    viewBox=\"");
  char viewBox[200];
  snprintf(viewBox, sizeof(viewBox), "%ld %ld %ld %ld",
           0L, lround(descender), lround(x / 64),
           lround(ascender - descender));
  svg->append(viewBox);
  svg->append("\">\n");

  if (false) {
    char buffer[200];
    snprintf(buffer, sizeof(buffer),
	     "  <rect fill=\"none\" stroke=\"blue\" stroke-width=\"10\" "
	     "x=\"%ld\" y=\"%ld\" width=\"%ld\" height=\"%ld\"/>\n",
	     0L, lround(descender), lround(x / 64),
	     lround(ascender - descender));
    svg->append(buffer);
  }

  svg->append(symbols);
  svg->append(uses);
  svg->append("</svg>\n");
  return true;
}

}  // namespace fonttest
