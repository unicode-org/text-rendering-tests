cmake_minimum_required(VERSION 3.0)

project(freetype)

add_library(freetype STATIC
    freetype2/src/autofit/autofit.c
    freetype2/src/base/ftbase.c
    freetype2/src/base/ftbbox.c
    freetype2/src/base/ftbitmap.c
    freetype2/src/base/ftdebug.c
    freetype2/src/base/ftfstype.c
    freetype2/src/base/ftgasp.c
    freetype2/src/base/ftglyph.c
    freetype2/src/base/ftinit.c
    freetype2/src/base/ftmm.c
    freetype2/src/base/ftpatent.c
    freetype2/src/base/ftstroke.c
    freetype2/src/base/ftsynth.c
    freetype2/src/base/ftsystem.c
    freetype2/src/base/fttype1.c
    freetype2/src/base/ftwinfnt.c
    freetype2/src/bdf/bdf.c
    freetype2/src/cff/cff.c
    freetype2/src/cid/type1cid.c
    freetype2/src/gzip/ftgzip.c
    freetype2/src/lzw/ftlzw.c
    freetype2/src/pcf/pcf.c
    freetype2/src/pfr/pfr.c
    freetype2/src/psaux/psaux.c
    freetype2/src/pshinter/pshinter.c
    freetype2/src/psnames/psnames.c
    freetype2/src/raster/raster.c
    freetype2/src/sdf/sdf.c
    freetype2/src/sfnt/sfnt.c
    freetype2/src/smooth/smooth.c
    freetype2/src/truetype/truetype.c
    freetype2/src/type1/type1.c
    freetype2/src/type42/type42.c
    freetype2/src/winfonts/winfnt.c
)

target_include_directories(freetype
    PUBLIC freetype2/include
    PRIVATE freetype2/include/freetype/config
)

target_compile_definitions(freetype
    PRIVATE -DFT_CONFIG_MODULES_H=<ftmodule.h>
    PRIVATE -DFT2_BUILD_LIBRARY=1
)
