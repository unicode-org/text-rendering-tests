[![Build Status](https://travis-ci.org/unicode-org/text-rendering-tests.svg)](https://travis-ci.org/unicode-org/text-rendering-tests)

# Text rendering tests

This is a very early draft for a test suite for text rendering.

It is not easy to correctly display text, so we founded this project
to help implementations to get this right.  Currently, the test suite
is still very much in its infancy, so please don’t be disappointed if
you don’t find much. Of course you are more than welcome to help; just
send a pull request.

```bash
$ git clone --recursive https://github.com/unicode-org/text-rendering-tests.git
$ cd text-rendering-tests
$ python check.py --output=report.html --engine=FreeStack
```


## Supported Platforms

Currently, the test suite supports only two OpenType implementations:

* With `--engine=FreeStack`, the tests are run on the free/libre
open-source text rendering stack with [FreeType](https://www.freetype.org/),
[HarfBuzz](https://www.freedesktop.org/wiki/Software/HarfBuzz/),
[FriBidi](https://www.fribidi.org/),
and [Raqm](https://github.com/HOST-Oman/libraqm). These libraries
are used by Linux, Android, ChromeOS, and many other systems.

* With `--engine=CoreText`, the tests are run on Apple’s CoreText.
This option will work only if you run the test suite on MacOS X.

If you’d like to test another OpenType implementation, please go ahead.


## Generated Reports

When you pass `--output=report.html`, the test suite will generate a
test report that explains what was tested, which tests have passed,
and which ones have failed. By clicking the following links, you can
also just look at the reports
for [FreeStack](https://raw.githack.com/unicode-org/text-rendering-tests/master/reports/FreeStack.html)
and [CoreText](https://raw.githack.com/unicode-org/text-rendering-tests/master/reports/CoreText.html) without running the test suite yourself.


## Test Cases

The test cases are defined in the [testcases](testcases/) directory.
It contains HTML snippets which describe each test, and define the
rendering parameters together with the expected result.

For each test case, the `check.py` script parses the HTML snippet to
extract the rendering parameters. Then, it runs a sub-process (written
in C++ and Objective C) that writes the observed rendering in SVG
format to Standard Output. Finally, the script checks whether the
expected rendering matches the observed result.  Currently, “matching”
is implemented by iterating over SVG paths, allowing for maximally
1 font design unit of difference.


## Contributing

Your contributions are very welcome; simply send pull requests via
GitHub.  A bot will ask you (on the GitHub review thread) to accept
[Unicode’s Contributor License Agreement](unicode_cla.pdf) by clicking
on a web form. Alternatively, if you prefer paper, you can also send a
signed paper copy of the agreement to Unicode.
