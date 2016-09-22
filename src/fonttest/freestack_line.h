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

#ifndef FONTTEST_FREESTACK_LINE_H_
#define FONTTEST_FREESTACK_LINE_H_

#include <ft2build.h>
#include FT_FREETYPE_H

#include "raqm.h"

#include "fonttest/font.h"

namespace fonttest {

class FreeStackLine {
 public:
  FreeStackLine(const std::string& text, const std::string& textLanguage,
                FT_Face font, double fontSize);
  ~FreeStackLine();
  bool RenderSVG(const std::string& idPrefix, std::string* svg);

 private:
  raqm_t* line_;
  FT_Face font_;
  double fontSize_;
};

}  // namespace fonttest

#endif  // FONTTEST_FREESTACK_LINE_H_
