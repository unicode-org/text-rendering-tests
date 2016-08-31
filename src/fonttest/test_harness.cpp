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
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "fonttest/avar_test.h"
#include "fonttest/font.h"
#include "fonttest/font_engine.h"
#include "fonttest/test_case.h"
#include "fonttest/test_harness.h"

namespace fonttest {

TestHarness::TestHarness(const std::vector<std::string>& options)
  : options_(options),
    engine_(FontEngine::Create(GetOption("--engine="))) {
  if (!engine_.get()) {
    PrintUsageAndExit();
  }
}

TestHarness::~TestHarness() {
}

void TestHarness::RunTest() {
  std::unique_ptr<TestCase> testcase;
  std::string name = GetOption("--testcase=");
  if (name == "AVAR-1") {
    testcase.reset(new AVARTest(this));
  }
  if (testcase.get()) {
    testcase->Run();
  } else {
    std::cerr << "unknown --testcase=" << name << std::endl;
    exit(1);
  }
}

Font* TestHarness::LoadFont(const std::string& filename, int faceIndex) {
  std::string path = GetOption("--fontdir=");
  if (!path.empty()) {
    path.append("/");
  }  
  path.append(filename);
  Font* font = engine_->LoadFont(path, faceIndex);
  if (!font) {
    std::cerr << "failed to load font: " << path << std::endl;
    exit(1);
  }
  return font;
}

const std::string TestHarness::GetOption(const std::string& flag) const {
  for (auto iter = options_.begin(); iter != options_.end(); ++iter) {
    if (iter->find(flag) == 0) {
      return iter->substr(flag.length());
    }
  }
  return "";
}

void TestHarness::PrintUsageAndExit() {
  std::cerr
    << "Usage: fonttest" << std::endl
    << "  --testcase=avar" << std::endl
    << "  --engine={FreeType/HarfBuzz, DirectWrite, CoreText}" << std::endl
    << "  --fontdir=path/to/testfonts" << std::endl;
  exit(1);
}

void TestHarness::StartTestCase(const std::string& testCase) {
  std::cout
    << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
    << "<TestCase name=\"" << testCase << "\"" << std::endl
    << "          xmlns=\"https://github.com/OpenType/fonttest\""
    << std::endl
    << "          xmlns:svg=\"http://www.w3.org/2000/svg\">"
    << std::endl;
}

void TestHarness::ReportGlyphOutline(Font* font,
                                     const std::string& glyphName, int glyphID,
                                     const FontVariation& variation) {
  std::string path, viewBox;
  font->GetGlyphOutline(glyphID, variation, &path, &viewBox);
  int val = 400;
  FontVariation::const_iterator it = variation.find("TEST");
  if (it != variation.end()) {
    val = it->second;
  }

#if 0
  std::cout << "  <GlyphOutline";
  if (!glyphName.empty()) {
    std::cout << " glyphName=\"" << glyphName << "\"";
  }
  std::cout << " glyphID=\"" << glyphID << "\"";
  if (!variation.empty()) {
    std::cout << " variation=\"";
    bool firstAxis = true;
    for (FontVariation::const_iterator iter = variation.begin();
	 iter != variation.end(); ++iter) {
      if (!firstAxis) {
	std::cout << ",";
      }
      firstAxis = false;
      std::cout << iter->first << ":" << iter->second;
    }
    std::cout << "\">" << std::endl;
  }
#endif

  std::cout
    << "    <td class=\"expected\" ft:id=\"AVAR-1/" << val
    << "\"" << std::endl
    << "        ft:render=\"⨁\" ft:font=\"TestAVAR.ttf\" ft:var=\"TEST:"
    << val << "\"" << std::endl
    << "      <svg viewBox=\"" << viewBox << "><g><path d=\"" << std::endl
    << "        ";
  for (auto iter = path.begin(); iter != path.end(); ++iter) {
    if (*iter != '\n') {
      std::cout << *iter;
    } else {
      std::cout << std::endl << "        ";
    }
  }
  std::cout << std::endl << "      \"></path></g></svg>" << std::endl
	    << "    </td>" << std::endl << std::endl;
}

void TestHarness::EndTestCase() {
  std::cout << "</TestCase>" << std::endl;
}

}  // namespace fonttest

