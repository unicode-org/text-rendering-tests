#!/usr/bin/python2.7
#
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

import argparse, os, re, subprocess
import xml.etree.ElementTree as etree


def build():
    subprocess.check_call(
        './src/third_party/gyp/gyp -f make --depth . '
        '--generator-output build  src/fonttest/fonttest.gyp'.split())
    subprocess.check_call(['make', '-s', '--directory', 'build'])


def check1(path, testcase, engine):
    success = True
    with open(path, 'r') as expected_file:
        expected = expected_file.read()
    expected_lines = [l + os.linesep for l in expected.splitlines()]
    command = ['./build/out/Default/fonttest',
               '--testcase=%s' % testcase,
               '--fontdir=fonts',
               '--engine=%s' % engine]
    observed = subprocess.check_output(command)
    observed_lines = [l + os.linesep for l in observed.splitlines()]
    if expected_lines != observed_lines:
        success = False
        observed_path = path.replace('expected',
                                     'observed-' + engine.replace('/', '-'))
        print
        for line in difflib.unified_diff(
                expected_lines, observed_lines,
                fromfile=path, tofile=observed_path):
            print line,
    return success


FONTTEST_NAMESPACE = '{https://github.com/OpenType/fonttest}'
FONTTEST_ID = FONTTEST_NAMESPACE + 'id'
FONTTEST_FONT = FONTTEST_NAMESPACE + 'font'
FONTTEST_RENDER = FONTTEST_NAMESPACE + 'render'
FONTTEST_VARIATION = FONTTEST_NAMESPACE + 'var'


class ConformanceChecker:
    def __init__(self, engine):
        self.engine = engine

    def check(self, testfile):
        doc = etree.parse(testfile).getroot()
        for e in doc.findall(".//*[@class='expected']"):
            testcase = e.attrib[FONTTEST_ID]
            font = os.path.join('fonts', e.attrib[FONTTEST_FONT])
            render = e.attrib.get(FONTTEST_RENDER)
            variation = e.attrib.get(FONTTEST_VARIATION)
            expected_svg = e.find('svg')
            self.normalize_svg(expected_svg)
            command = ['build/out/Default/fonttest', '--font=' + font,
                       '--testcase=' + testcase, '--engine=' + self.engine]
            if render: command.append('--render=' + render)
            if variation: command.append('--variation=' + variation)
            observed = subprocess.check_output(command)
            observed_svg = etree.fromstring(observed)
            self.normalize_svg(observed_svg)
            expected_str = \
                etree.tostring(expected_svg, encoding='utf-8').strip()
            observed_str = \
                etree.tostring(observed_svg, encoding='utf-8').strip()
            print testcase, expected_str == observed_str

    def normalize_svg(self, svg):
        strip_path = lambda p: re.sub(r'\s+', ' ', p).strip()
        for path in svg.findall(".//path[@d]"):
            path.attrib['d'] = strip_path(path.attrib['d'])


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--engine',
                        choices=['FreeStack', 'CoreText', 'DirectWrite'],
                        default='FreeStack')
    args = parser.parse_args()
    build()
    checker = ConformanceChecker(engine=args.engine)
    for filename in os.listdir('testcases'):
        if not filename.endswith('.html'): continue
        checker.check(os.path.join('testcases', filename))


if __name__ == '__main__':
    main()
