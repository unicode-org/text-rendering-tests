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
#include FT_IMAGE_H
#include FT_OUTLINE_H

#include "fonttest/freestack_path.h"

namespace fonttest {

FreeTypePathConverter::FreeTypePathConverter() {
}

FreeTypePathConverter::~FreeTypePathConverter() {
}

std::string FreeTypePathConverter::Convert(FT_Outline* outline) {
  path_.clear();
  start_.x = start_.y = 0;
  closed_ = true;

  FT_Outline_Funcs callbacks;
  callbacks.move_to = &FreeTypePathConverter::MoveToCallback;
  callbacks.line_to = &FreeTypePathConverter::LineToCallback;
  callbacks.conic_to = &FreeTypePathConverter::QuadToCallback;
  callbacks.cubic_to = &FreeTypePathConverter::CurveToCallback;
  callbacks.shift = 0;
  callbacks.delta = 0;
  FT_Error error =
      FT_Outline_Decompose(outline, &callbacks, static_cast<void*>(this));
  if (error) {
    std::cerr << "FT_Outline_Decompose() failed; error: " << error
	      << std::endl;
    exit(1);
  }
  if (!closed_) {
    path_.append("Z\n");
  }
  return path_;
}

void FreeTypePathConverter::MoveTo(const FT_Vector& to) {
  if (!closed_) {
    path_.append("Z\n");
  }
  char buffer[200];
  snprintf(buffer, sizeof(buffer), "M %ld,%ld\n", to.x, to.y);
  path_.append(buffer);
  start_ = to;
  closed_ = false;
}

void FreeTypePathConverter::LineTo(const FT_Vector& to) {
  if (to.x == start_.x && to.y == start_.y) {
    path_.append("Z\n");
    closed_ = true;
    return;
  }
  char buffer[200];
  snprintf(buffer, sizeof(buffer), "L %ld,%ld\n", to.x, to.y);
  path_.append(buffer);
  closed_ = false;
}

void FreeTypePathConverter::QuadTo(const FT_Vector& control,
                                   const FT_Vector& to) {
  char buffer[200];
  snprintf(buffer, sizeof(buffer), "Q %ld,%ld %ld,%ld\n",
	   control.x, control.y, to.x, to.y);
  path_.append(buffer);
  closed_ = false;
}

void FreeTypePathConverter::CurveTo(const FT_Vector& control1,
                                    const FT_Vector& control2,
                                    const FT_Vector& to) {
  char buffer[200];
  snprintf(buffer, sizeof(buffer), "C %ld,%ld %ld,%ld %ld,%ld\n",
	   control1.x, control1.y, control2.x, control2.y, to.x, to.y);
  path_.append(buffer);
  closed_ = false;
}


int FreeTypePathConverter::MoveToCallback(const FT_Vector* to, void* data) {
  if (to && data) {
    reinterpret_cast<FreeTypePathConverter*>(data)->MoveTo(*to);
    return 0;
  } else {
    return 1;
  }
}

int FreeTypePathConverter::LineToCallback(const FT_Vector* to, void* data) {
  if (to && data) {
    reinterpret_cast<FreeTypePathConverter*>(data)->LineTo(*to);
    return 0;
  } else {
    return 1;
  }
}

int FreeTypePathConverter::QuadToCallback(const FT_Vector* control,
					  const FT_Vector* to,
					  void* data) {
  if (control && to && data) {
    reinterpret_cast<FreeTypePathConverter*>(data)->QuadTo(*control, *to);
    return 0;
  } else {
    return 1;
  }
}

int FreeTypePathConverter::CurveToCallback(const FT_Vector* control1,
					   const FT_Vector* control2,
					   const FT_Vector* to,
					   void* data) {
  if (control1 && control2 && to && data) {
    FreeTypePathConverter* c = reinterpret_cast<FreeTypePathConverter*>(data);
    c->CurveTo(*control1, *control2, *to);
    return 0;
  } else {
    return 1;
  }
}

}  // namespace fonttest
