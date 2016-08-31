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

#include "fonttest/avar_test.h"
#include "fonttest/font.h"
#include "fonttest/test_case.h"
#include "fonttest/test_harness.h"

namespace fonttest {

AVARTest::AVARTest(TestHarness* harness)
  : TestCase(harness),
    font_(harness->LoadFont("TestAVAR.ttf", 0)) {
}

AVARTest::~AVARTest() {
}

void AVARTest::Run() {
  TestHarness* harness = GetHarness();
  harness->StartTestCase("AVAR-1");
  for (double value = 100; value <= 900; value += 50) {
    FontVariation variation;
    variation["TEST"] = value;
    harness->ReportGlyphOutline(font_.get(), "uni2A01", 1, variation);
  }
  harness->EndTestCase();
}

}  // namespace fonttest
