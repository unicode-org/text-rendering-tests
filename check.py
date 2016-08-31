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

import argparse, codecs, difflib, os, re, subprocess, sys
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
FONTTEST_VAR = FONTTEST_NAMESPACE + 'var'


def check(testfile, engine):
    doc = etree.parse(testfile).getroot()
    for e in doc.findall(".//*[@class='expected']"):
        testcase = e.attrib[FONTTEST_ID]
        font = os.path.join('fonts', e.attrib[FONTTEST_FONT])
        render = e.attrib.get(FONTTEST_RENDER)
        var = e.attrib.get(FONTTEST_VAR)
        expected_svg = e.find('svg')
        for path in expected_svg.findall(".//path[@d]"):
            path.attrib['d'] = strip_svg_path(path.attrib['d'])
        command = ['build/Default/out/fonttest', '--font=' + font,
                   '--testcase=' + testcase]
        if render: command.append('--render=' + render)
        if var: command.append('--var=' + var)
        print command
        #print id, font, render, var, etree.tostring(expected_svg, encoding='utf-8')
    pass


def strip_svg_path(path):
    return re.sub(r'\s+', ' ', path).strip()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--engine', choices=['free', 'coretext'],
                        default='free')
    args = parser.parse_args()
    #build()
    for filename in os.listdir('testcases'):
        if not filename.endswith('.html'): continue
        check(os.path.join('testcases', filename), args.engine)
    return
    build()
    engines = ['FreeType/HarfBuzz']
    native_engine = {'Darwin': 'CoreText'}.get(os.uname()[0])
    if native_engine:
        engines.append(native_engine)
    successes, failures = set(), set()
    for engine in engines:
        for filename in os.listdir('expected'):
            if not filename.endswith('.xml'):
                continue
            path = os.path.join('expected', filename)
            expected_xml = etree.parse(path).getroot()
            testcase = expected_xml.attrib['name']
            if check(path, testcase, engine):
                successes.add(testcase + ' ' + engine)
            else:
                failures.add(testcase + ' ' + engine)
    for s in sorted(successes): print('PASS ' + s)
    for f in sorted(failures):  print('FAIL ' + f)
    if len(failures) == 0:
        print('All tests have succeeded.')
        sys.exit(0)
    else:
        print('%d tests have failed.' % len(failures))
        sys.exit(1)


if __name__ == '__main__':
    main()
