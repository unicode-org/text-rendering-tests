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

#include "fonttest/font_engine.h"
#include "fonttest/fthb_engine.h"

#ifdef HAVE_CORETEXT
#  include "fonttest/coretext_engine.h"
#endif

namespace fonttest {

FontEngine* FontEngine::Create(const std::string& engineName) {
  if (engineName == "FreeType/HarfBuzz") {
    return new FreeTypeHarfBuzzEngine();
  }

#ifdef HAVE_CORETEXT
  if (engineName == "CoreText") {
    return new CoreTextEngine();
  }
#endif  // HAVE_CORETEXT

  if (engineName == "DirectWrite") {
    // TODO: Implement a DirectWrite engine.
  }

  return NULL;
}

}  // namespace fonttest
