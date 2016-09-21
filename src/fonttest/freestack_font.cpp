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

FT_Face FreeStackFont::GetFace(double size, const FontVariation& variation) {
  FT_Fixed fixedSize = static_cast<FT_Fixed>(size * 64 + 0.5);
  FT_Error error = FT_Set_Char_Size(face_, fixedSize, fixedSize, 0, 0);
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

  return face_;
}

void FreeStackFont::GetGlyphOutline(int glyphID,
                                    const FontVariation& variation,
                                    std::string* path,
                                    std::string* viewBox) {
  FT_Face face = GetFace(1000.0, variation);
  FT_Error error =
      FT_Load_Glyph(face, glyphID, FT_LOAD_NO_HINTING|FT_LOAD_NO_BITMAP);
  if (error) {
    std::cerr << "FT_Load_Glyph() failed; error: " << error << std::endl;
    exit(1);
  }

  if (!face->glyph) {
    std::cerr << "FT_Load_Glyph() did not load a glyph" << std::endl;
    exit(1);
  }

  FT_Vector transform;
  transform.x = transform.y = 0;
  FreeTypePathConverter converter(transform);
  path->assign(converter.Convert(&face->glyph->outline));
  char buffer[200];
  snprintf(buffer, sizeof(buffer), "%ld %ld %ld %ld",
           0L, lround(face->descender),
           lround(face->glyph->metrics.horiAdvance / 64),
           lround(face->height));
  viewBox->assign(buffer);
}

}  // namespace fonttest
