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
#
from __future__ import unicode_literals
import itertools


def is_similar(a, b, maxDelta):
    if (a is not b) and (a is None or b is None):
        return False
    if a.tag != b.tag:
        return False
    for name, valueA in a.attrib.items():
        valueB = b.attrib.get(name)
        if name in ('d', 'viewBox', 'x', 'y'):
            if not is_similar_path(valueA, valueB, maxDelta):
                return False
        else:
            if valueA != valueB:
                return False
    for childA, childB in \
            itertools.izip_longest(a.getchildren(), b.getchildren()):
        if not is_similar(childA, childB, maxDelta):
            return False
    return True


def is_similar_path(a, b, maxDelta):
    for itemA, itemB in itertools.izip_longest(parse_path(a), parse_path(b)):
        if (itemA is None or itemB is None):
            return False
        try:
            if abs(float(itemA) - float(itemB)) > maxDelta:
                return False
        except ValueError:
            if itemA != itemB:
                return False
    return True


# http://codereview.stackexchange.com/a/88051
def parse_path(path_data):
    digit_exp = '0123456789eE'
    comma_wsp = ', \t\n\r\f\v'
    drawto_command = 'MmZzLlHhVvCcSsQqTtAa'
    sign = '+-'
    exponent = 'eE'
    float = False
    entity = ''
    for char in path_data:
        if char in digit_exp:
            entity += char
        elif char in comma_wsp and entity:
            yield entity
            float = False
            entity = ''
        elif char in drawto_command:
            if entity:
                yield entity
                float = False
                entity = ''
            yield char
        elif char == '.':
            if float:
                yield entity
                entity = '.'
            else:
                entity += '.'
                float = True
        elif char in sign:
            if entity and entity[-1] not in exponent:
                yield entity
                float = False
                entity = char
            else:
                entity += char
    if entity:
        yield entity
