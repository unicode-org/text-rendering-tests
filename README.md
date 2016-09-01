# FontTest

This is an early sketch for how an OpenType test suite might look like.

```bash
$ git clone --recursive https://github.com/brawer/fonttest.git && cd fonttest
$ python check.py --output=report.html --engine=FreeStack
PASS
```

With `--engine=FreeStack`, the tests are run on the free/libre
open-source text rendering stack that consists of the FreeType, HarfBuzz,
and Raqm. If you pass `--engine=CoreText`, the tests are run on Apple’s
CoreText.

If you pass `--output=report.html`, the test suite will generate a
[test report](reports/fake-fail.html) that shows what was tested,
which tests have passed, and which ones have failed.

The test cases are defined in the [testcases](testcases/) directory.
It contains HTML snippets which describe each test, and define the
rendering parameters together with the expected result.

For each test case, the `check.py` script parses the HTML snippet to
extract the rendering parameters. Then, it runs a sub-process (written
in C++ and Objective C) that writes the observed rendering in SVG
format to Standard Output. Finally, the script checks whether the
expected rendering matches the observed result.  Currently, “matching”
is implemented as exact string equality on the SVG file; if needed,
this could of course be made resilient to small rounding differences.
