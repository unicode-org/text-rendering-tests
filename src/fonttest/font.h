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

#ifndef FONTTEST_FONT_H_
#define FONTTEST_FONT_H_

#include <map>
#include <string>

namespace fonttest {

typedef std::map<std::string, double> FontVariation;  // WGHT -> 400.0

class Font {
 public:
  virtual ~Font() {}

  // Returns the path of a glyph outline, in SVG path format.
  // For example, "M 100 100 L 300 100 L 200 300 Z",
  virtual void GetGlyphOutline(int glyphID, const FontVariation& variation,
                               std::string* path, std::string* viewBox) = 0;
};

}  // namespace fonttest

#endif  // FONTTEST_FONT_H_
