# Copyright 2016 Unicode Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
{
  'targets': [
    {
      'target_name': 'fonttest',
      'type': 'executable',
      'cflags': ['-std=c++11'],
      'sources': [
        'main.cpp',
        'font_engine.cpp',
        'freestack_engine.cpp',
        'freestack_font.cpp',
        'freestack_path.cpp',
        'test_harness.cpp',
      ],
      'include_dirs': ['..'],
      'dependencies': [
        '../third_party/freetype/freetype.gyp:freetype',
      ],
      'conditions': [
        [
          'OS=="mac"', {
            'defines': [ 'HAVE_CORETEXT' ],
            'sources': [
              'coretext_engine.mm',
              'coretext_font.mm',
              'coretext_line.mm',
              'coretext_path.mm',
            ],
            'link_settings': {
              'libraries': [
                '$(SDKROOT)/System/Library/Frameworks/CoreFoundation.framework',
                '$(SDKROOT)/System/Library/Frameworks/CoreGraphics.framework',
                '$(SDKROOT)/System/Library/Frameworks/CoreText.framework',
                '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
              ],
            },
          },
        ],
      ],
    },
  ],
  'target_defaults': {
    'xcode_settings': {
      'GCC_VERSION': 'com.apple.compilers.llvm.clang.1_0',
      'CLANG_CXX_LANGUAGE_STANDARD': 'c++11',
      'CLANG_CXX_LIBRARY': 'libc++',
    },
  },
}
