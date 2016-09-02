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

#include "fonttest/font.h"
#include "fonttest/font_engine.h"
#include "fonttest/test_harness.h"

namespace fonttest {

// Helper methods for parsing command-line arguments.
static void TrimWhitespace(std::string* str);
static void SplitString(const std::string& text, char sep,
                        std::vector<std::string>* result);

static void ParseVariationSpec(const std::string& spec,
                               FontVariation* variation) {
  if (spec.empty()) {
    return;
  }

  std::vector<std::string> v;
  SplitString(spec, ';', &v);
  for (const std::string& item : v) {
    std::vector<std::string> keyValue;
    SplitString(item, ':', &keyValue);
    if (keyValue.size() != 2) {
      std::cerr << "malformed --variation=" << spec << std::endl;
      exit(1);
    }
    std::string key = keyValue[0];
    std::string value = keyValue[1];
    TrimWhitespace(&key);
    TrimWhitespace(&value);
    (*variation)[key] = std::atof(value.c_str());
  }
}

TestHarness::TestHarness(const std::vector<std::string>& options)
  : options_(options),
    engine_(FontEngine::Create(GetOption("--engine="))) {
  if (!engine_.get()) {
    PrintUsageAndExit();
  }

  std::string fontPath = GetOption("--font=");
  int fontIndex = 0;
  if (!fontPath.empty()) {
    font_.reset(engine_->LoadFont(fontPath, fontIndex));
    if (!font_.get()) {
      std::cerr << "failed to load font: " << fontPath << std::endl;
      exit(1);
    }
  }
}

TestHarness::~TestHarness() {
}

void TestHarness::Run() {
  FontVariation fontVariation;
  const std::string variationSpec = GetOption("--variation=");
  ParseVariationSpec(variationSpec, &fontVariation);
  const std::string text = GetOption("--render=");
  const std::string textLanguage = GetOption("--textLanguage=");
  const double fontSize = 1000.0;
  std::string svg;
  engine_->RenderSVG(text, textLanguage, font_.get(), fontSize, fontVariation,
                     &svg);
  std::cout << svg;
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
    << "  --render=Text" << std::endl
    << "  --variation=WGHT:700;WDTH:120" << std::endl
    << "  --testcase=AVAR-1/789" << std::endl
    << "  --engine={FreeStack, DirectWrite, CoreText}" << std::endl
    << "  --font=path/to/testfont.otf" << std::endl;
  exit(1);
}

void SplitString(const std::string& text, char sep,
                 std::vector<std::string>* result) {
  std::size_t start = 0, limit = 0;
  while ((limit = text.find(sep, start)) != std::string::npos) {
    result->push_back(text.substr(start, limit - start));
    start = limit + 1;
  }
  result->push_back(text.substr(start));
}

void TrimWhitespace(std::string* str) {
  static const char* whitespace = " \t\f\v\n\r";
  const std::size_t start = str->find_first_not_of(whitespace);
  if (start == std::string::npos) {
    str->clear();
    return;
  }
  str->substr(start).swap(*str);
  const std::size_t end = str->find_last_not_of(whitespace);
  if (end != std::string::npos) {
    str->erase(end + 1);
  }
}

}  // namespace fonttest

