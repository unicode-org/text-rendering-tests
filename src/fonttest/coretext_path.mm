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

#include <CoreGraphics/CoreGraphics.h>

#include "fonttest/coretext_path.h"

namespace fonttest {

CoreTextPath::CoreTextPath(CGPathRef path) {
  CGPathApply(path, reinterpret_cast<void*>(this),
              &CoreTextPath::VisitPathElement);
}

CoreTextPath::~CoreTextPath() {
}

const std::string& CoreTextPath::ToSVGPath() {
  return svgPath_;
}

void CoreTextPath::VisitPathElement(void* data, const CGPathElement* element) {
  CoreTextPath* self = static_cast<CoreTextPath*>(data);
  std::string* svg = &self->svgPath_;

  char buffer[100];
  long x, y, ax, ay, bx, by;
  switch (element->type) {
  case kCGPathElementMoveToPoint:
    x = static_cast<long>(element->points[0].x);
    y = static_cast<long>(element->points[0].y);
    snprintf(buffer, sizeof(buffer), "M %ld,%ld\n", x, y);
    svg->append(buffer);
    break;

  case kCGPathElementAddLineToPoint:
    x = static_cast<long>(element->points[0].x);
    y = static_cast<long>(element->points[0].y);
    snprintf(buffer, sizeof(buffer), "L %ld,%ld\n", x, y);
    svg->append(buffer);
    break;

  case kCGPathElementAddQuadCurveToPoint:
    ax = static_cast<long>(element->points[0].x);
    ay = static_cast<long>(element->points[0].y);
    x = static_cast<long>(element->points[1].x);
    y = static_cast<long>(element->points[1].y);
    snprintf(buffer, sizeof(buffer), "Q %ld,%ld %ld,%ld\n", ax, ay, x, y);
    svg->append(buffer);
    break;

  case kCGPathElementAddCurveToPoint:
    ax = static_cast<long>(element->points[0].x);
    ay = static_cast<long>(element->points[0].y);
    bx = static_cast<long>(element->points[1].x);
    by = static_cast<long>(element->points[1].y);
    x = static_cast<long>(element->points[2].x);
    y = static_cast<long>(element->points[2].y);
    snprintf(buffer, sizeof(buffer), "C %ld,%ld %ld,%ld %ld,%ld\n",
             ax, ay, bx, by, x, y);
    svg->append(buffer);
    break;

  case kCGPathElementCloseSubpath:
    svg->append("Z\n");
    break;
  }
}

}  // namespace fonttest
