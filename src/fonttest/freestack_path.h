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

#ifndef FONTTEST_FREESTACK_PATH_H_
#define FONTTEST_FREESTACK_PATH_H_

#include <string>

#include <ft2build.h>
#include FT_IMAGE_H
#include FT_TYPES_H

namespace fonttest {

class FreeTypePathConverter {
 public:
  FreeTypePathConverter();
  ~FreeTypePathConverter();
  std::string Convert(FT_Outline* outline);

 private:
  void MoveTo(const FT_Vector& to);
  void LineTo(const FT_Vector& to);
  void QuadTo(const FT_Vector& control, const FT_Vector& to);
  void CurveTo(const FT_Vector& control1, const FT_Vector& control2,
               const FT_Vector& to);

  static int MoveToCallback(const FT_Vector* to, void* data);
  static int LineToCallback(const FT_Vector* to, void* data);
  static int QuadToCallback(const FT_Vector* control,
                            const FT_Vector* to, void* data);
  static int CurveToCallback(const FT_Vector* control1,
                             const FT_Vector* control2,
                             const FT_Vector* to, void* data);

  std::string path_;
  FT_Vector start_;
  bool closed_;
};

}  // namespace fonttest

#endif  // FONTTEST_FREESTACK_PATH_H_
