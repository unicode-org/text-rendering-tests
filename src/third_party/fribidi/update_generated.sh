#!/bin/bash

BUILD_DIR=build/out/Default
GENERATED=src/third_party/fribidi/generated
UNIDATA=src/third_party/fribidi/fribidi/gen.tab/unidata
MAX_DEPTH=3

rm -rf $GENERATED
mkdir $GENERATED

make -s --directory build gen-unicode-version
$BUILD_DIR/gen-unicode-version \
    $UNIDATA/*.txt >$GENERATED/fribidi-unicode-version.h

make -s --directory build gen-arabic-shaping-tab
$BUILD_DIR/gen-arabic-shaping-tab \
    $MAX_DEPTH $UNIDATA/UnicodeData.txt >$GENERATED/arabic-shaping.tab.i

make -s --directory build gen-bidi-type-tab
$BUILD_DIR/gen-bidi-type-tab \
    $MAX_DEPTH $UNIDATA/UnicodeData.txt >$GENERATED/bidi-type.tab.i

make -s --directory build gen-mirroring-tab
$BUILD_DIR/gen-mirroring-tab \
    $MAX_DEPTH $UNIDATA/BidiMirroring.txt >$GENERATED/mirroring.tab.i

make -s --directory build gen-joining-type-tab
$BUILD_DIR/gen-joining-type-tab \
    $MAX_DEPTH $UNIDATA/UnicodeData.txt $UNIDATA/ArabicShaping.txt \
    >$GENERATED/joining-type.tab.i

make -s --directory build gen-brackets-tab
$BUILD_DIR/gen-brackets-tab \
    $MAX_DEPTH $UNIDATA/BidiBrackets.txt $UNIDATA/UnicodeData.txt \
    >$GENERATED/brackets.tab.i

make -s --directory build gen-brackets-type-tab
$BUILD_DIR/gen-brackets-type-tab \
    $MAX_DEPTH $UNIDATA/BidiBrackets.txt \
    >$GENERATED/brackets-type.tab.i
