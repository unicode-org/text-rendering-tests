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
[test report](reports/fake-for-illustration-only.html) that shows
what was tested, which tests have passed, and which ones have failed.

The test cases are defined in the [testcases](testcases/) directory.
It contains HTML snippets which describe each test, and define the
rendering rendering parameters together with the expected result.
The `check.py` script parses the test cases, runs a sub-process to
render the test case. It then checks whether the expected rendering
matches the observed result, and collects it for generating the
final report.

