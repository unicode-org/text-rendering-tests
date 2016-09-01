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

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>

#include <ft2build.h>
#include FT_MULTIPLE_MASTERS_H

#include "fonttest/font.h"
#include "fonttest/freestack_font.h"
#include "fonttest/freestack_path.h"

namespace fonttest {

FreeStackFont::FreeStackFont(FT_Face face)
  : face_(face) {
}

FreeStackFont::~FreeStackFont() {
  FT_Done_Face(face_);
}

static std::string TagToString(FT_ULong tag) {
  char s[5];
  s[0] = static_cast<char>((tag & 0xff000000) >> 24);
  s[1] = static_cast<char>((tag & 0x00ff0000) >> 16);
  s[2] = static_cast<char>((tag & 0x0000ff00) >> 8);
  s[3] = static_cast<char>((tag & 0x000000ff) >> 0);
  s[4] = 0;
  if (s[3] == 0x20) {
    s[3] = 0;
    if (s[2] == 0x20) {
      s[2] = 0;
      if (s[1] == 0x20) {
        s[1] = 0;
      }
    }
  }
  return std::string(s);
}

void FreeStackFont::GetGlyphOutline(int glyphID,
                                    const FontVariation& variation,
                                    std::string* path,
                                    std::string* viewBox) {
  FT_UShort upem = face_->units_per_EM;
  FT_Error error = FT_Set_Char_Size(face_, upem, upem, 0, 0);
  if (error) {
    std::cerr << "FT_Set_Char_Size() failed; error: " << error << std::endl;
    exit(1);
  }

  FT_MM_Var* mmvar = NULL;
  FT_Get_MM_Var(face_, &mmvar);
  if (mmvar) {
    FT_Fixed coords[mmvar->num_axis];
    for (FT_UInt axisIndex = 0; axisIndex < mmvar->num_axis; ++axisIndex) {
      const FT_Var_Axis& axis = mmvar->axis[axisIndex];
      coords[axisIndex] = axis.def;
      FontVariation::const_iterator iter =
          variation.find(TagToString(axis.tag));
      if (iter != variation.end()) {
        coords[axisIndex] =
          static_cast<FT_Fixed>(iter->second * 65536.0 + 0.5);
      }
    }
    error = FT_Set_Var_Design_Coordinates(face_, mmvar->num_axis, coords);
    if (error) {
      std::cerr << "FT_Set_Var_Design_Coordinates() failed; error: "
                << error << std::endl;
      exit(1);
    }
    free(static_cast<void*>(mmvar));
  }

  error = FT_Load_Glyph(face_, glyphID, FT_LOAD_NO_HINTING|FT_LOAD_NO_BITMAP);
  if (error) {
    std::cerr << "FT_Load_Glyph() failed; error: " << error << std::endl;
    exit(1);
  }

  if (!face_->glyph) {
    std::cerr << "FT_Load_Glyph() did not load a glyph" << std::endl;
    exit(1);
  }

  FreeTypePathConverter converter;
  path->assign(converter.Convert(&face_->glyph->outline));
  char buffer[200];
  snprintf(buffer, sizeof(buffer), "%ld %ld %ld %ld",
           static_cast<long>(0),
           static_cast<long>(face_->descender),
           static_cast<long>(face_->glyph->metrics.horiAdvance),
           static_cast<long>(face_->height));
  viewBox->assign(buffer);
}

}  // namespace fonttest
