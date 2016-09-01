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

#ifndef FONTTEST_FREESTACK_FONT_H_
#define FONTTEST_FREESTACK_FONT_H_

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TYPES_H

#include "fonttest/font.h"

namespace fonttest {

class FreeStackFont : public Font {
 public:
  FreeStackFont(FT_Face face);
  ~FreeStackFont();

  virtual void GetGlyphOutline(int glyphID, const FontVariation& variation,
                               std::string* path, std::string* viewBox);

 private:
  FT_Face face_;
};

}  // namespace fonttest

#endif  // FONTTEST_FREESTACK_FONT_H_
