[![Build Status](https://travis-ci.org/unicode-org/text-rendering-tests.svg)](https://travis-ci.org/unicode-org/text-rendering-tests)

# Text rendering tests

This is a test suite for text rendering engines. It is not easy to correctly
display text, so we founded this project to help implementations to
get this right.

```bash
$ git clone --recursive https://github.com/unicode-org/text-rendering-tests.git
$ cd text-rendering-tests
$ for engine in CoreText FreeStack TehreerStack fontkit OpenType.js ; do python check.py --engine=$engine --output=reports/$engine.html  ; done
```


## Supported Platforms

Currently, the test suite supports four OpenType implementations:

* With `--engine=FreeStack`, the tests are run on the free/libre
open-source text rendering stack with [FreeType](https://www.freetype.org/),
[HarfBuzz](https://www.freedesktop.org/wiki/Software/HarfBuzz/),
[FriBidi](https://www.fribidi.org/),
and [Raqm](https://github.com/HOST-Oman/libraqm). These libraries
are used by Linux, Android, ChromeOS, and many other systems.
— [Test report for FreeStack](https://rawgit.com/unicode-org/text-rendering-tests/master/reports/FreeStack.html).

* With `--engine=CoreText`, the tests are run on Apple’s CoreText.
This option will work only if you run the test suite on MacOS X.
— [Test report for CoreText](https://rawgit.com/unicode-org/text-rendering-tests/master/reports/CoreText.html).

* With `--engine=TehreerStack`, the tests are run on an open-source
text rendering stack consisting of [FreeType](https://www.freetype.org/),
[SheenBidi](https://github.com/Tehreer/SheenBidi), and
[SheenFigure](https://github.com/Tehreer/SheenFigure).
— [Test report for TehreerStack](https://rawgit.com/unicode-org/text-rendering-tests/master/reports/TehreerStack.html).

* With `--engine=fontkit`, the tests are run on
[fontkit](http://github.com/devongovett/fontkit), a JavaScript font engine.
— [Test report for fontkit](https://rawgit.com/unicode-org/text-rendering-tests/master/reports/fontkit.html).

* With `--engine=OpenType.js`, the tests are run using [OpenType.js](https://github.com/nodebox/opentype.js), another JavaScript font engine.
— [Test report for OpenType.js](https://rawgit.com/unicode-org/text-rendering-tests/master/reports/OpenType.js.html).

It’s trivial to test other implementations; simply write a small
wrapper tool. For the [Go font
library](https://godoc.org/golang.org/x/image/font/sfnt), see
[here](https://github.com/golang/go/issues/20208). For the [Rust font
library](https://github.com/google/font-rs), see
[here](https://github.com/google/font-rs/issues/17).


## Test Cases

The test cases are defined in the [testcases](testcases/) directory.
It contains HTML snippets which describe each test, and define the
rendering parameters together with the expected result.

For each test case, the `check.py` script parses the HTML snippet to
extract the rendering parameters. Then, it runs a sub-process (written
in C++, Objective C or JavaScript depending on the tested
implementation) that writes the observed rendering in SVG format to
Standard Output. Finally, the script checks whether the expected
rendering matches the observed result.  Currently, “matching” is
implemented by iterating over SVG paths, allowing for maximally 1 font
design unit of difference.


## Contributing

Your contributions are very welcome; simply send pull requests via
GitHub.  A bot will ask you (on the GitHub review thread) to accept
[Unicode’s Contributor License Agreement](unicode_cla.pdf) by clicking
on a web form. Alternatively, if you prefer paper, you can also send a
signed paper copy of the agreement to Unicode.
