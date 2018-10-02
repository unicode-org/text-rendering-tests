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

#ifndef FONTTEST_TEST_HARNESS_H_
#define FONTTEST_TEST_HARNESS_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace fonttest {

class Font;
class FontEngine;

typedef std::map<std::string, double> FontVariation;  // WGHT -> 400.0

class TestHarness {
 public:
  TestHarness(const std::vector<std::string>& options);
  ~TestHarness();
  void Run();

 private:
  bool HasOption(const std::string& flag) const;
  const std::string GetOption(const std::string& flag) const;
  void PrintUsageAndExit();

  const std::vector<std::string> options_;
  std::unique_ptr<FontEngine> engine_;
  std::unique_ptr<Font> font_;
};

}  // namespace fonttest

#endif  // FONTTEST_TEST_HARNESS_H_
