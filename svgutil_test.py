#!/usr/bin/python2.7
# -*- coding: utf-8 -*-
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

from __future__ import unicode_literals
import xml.etree.ElementTree as etree
import unittest

import svgutil


SVG_A = etree.fromstring('''
    <svg viewBox="0 -292 518 1360">
        <g><path d="M 83,424 Q 56,458 56,517 Z"/></g>
    </svg>''')

SVG_B = etree.fromstring('''
    <svg viewBox="0 -292 520 1360">
        <g><path d="M 83,424 Q 56,458 56,517 Z"/></g>
    </svg>''')

SVG_C = etree.fromstring('''
    <svg viewBox="0 -292 518 1360">
        <g><path d="M 83,424 Q 56,458 57.7,517 Z"/></g>
    </svg>''')


class TestSVGHandling(unittest.TestCase):
    def test_is_similar(self):
        self.assertTrue(svgutil.is_similar(SVG_A, SVG_A, maxDelta=0.0))
        self.assertFalse(svgutil.is_similar(SVG_A, SVG_B, maxDelta=1.0))
        self.assertTrue(svgutil.is_similar(SVG_A, SVG_B, maxDelta=5.0))
        self.assertFalse(svgutil.is_similar(SVG_A, SVG_C, maxDelta=1.0))
        self.assertTrue(svgutil.is_similar(SVG_A, SVG_C, maxDelta=5.0))

    def test_is_similar_path(self):
        self.assertTrue(svgutil.is_similar_path('M1,2 L3,4', 'M1,2 L4,4', 1))
        self.assertFalse(svgutil.is_similar_path('M1,2 L3,4', 'M1,2 L1,4', 1))

    def test_parse_path(self):
        self.assertEqual(
          ' '.join(svgutil.parse_path('M 83.7,424 Q56,458Z')),
          'M 83.7 424 Q 56 458 Z')


if __name__ == '__main__':
    unittest.main()
