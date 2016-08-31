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

#ifndef FONTTEST_TEST_CASE_H_
#define FONTTEST_TEST_CASE_H_

namespace fonttest {

class TestHarness;

class TestCase {
 public:
  TestCase(TestHarness* harness);
  virtual ~TestCase();
  TestHarness* GetHarness() const { return harness_; }
  virtual void Run() = 0;

 private:
  TestHarness* harness_;
};

}  // namespace fonttest

#endif  // FONTTEST_TEST_CASE_H_
