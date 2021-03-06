cmake_minimum_required(VERSION 3.0)

project(freetype)

add_library(freetype STATIC
    freetype2/src/autofit/autofit.c
    freetype2/src/base/ftbase.c
    freetype2/src/base/ftbbox.c
    freetype2/src/base/ftbitmap.c
    freetype2/src/base/ftdebug.c
    freetype2/src/base/ftfntfmt.c
    freetype2/src/base/ftfstype.c
    freetype2/src/base/ftgasp.c
    freetype2/src/base/ftglyph.c
    freetype2/src/base/ftinit.c
    freetype2/src/base/ftlcdfil.c
    freetype2/src/base/ftmm.c
    freetype2/src/base/ftpatent.c
    freetype2/src/base/ftstroke.c
    freetype2/src/base/ftsynth.c
    freetype2/src/base/ftsystem.c
    freetype2/src/base/fttype1.c
    freetype2/src/base/ftwinfnt.c
    freetype2/src/bdf/bdf.c
    freetype2/src/bdf/bdflib.c
    freetype2/src/bdf/bdfdrivr.c
    freetype2/src/cff/cff.c
    freetype2/src/cid/cidgload.c
    freetype2/src/cid/cidparse.c
    freetype2/src/cid/cidload.c
    freetype2/src/cid/cidriver.c
    freetype2/src/cid/cidobjs.c
    freetype2/src/cid/type1cid.c
    freetype2/src/gzip/ftgzip.c
    freetype2/src/lzw/ftlzw.c
    freetype2/src/lzw/ftzopen.c
    freetype2/src/pcf/pcf.c
    freetype2/src/pcf/pcfread.c
    freetype2/src/pcf/pcfdrivr.c
    freetype2/src/pcf/pcfutil.c
    freetype2/src/pfr/pfr.c
    freetype2/src/pfr/pfrload.c
    freetype2/src/pfr/pfrcmap.c
    freetype2/src/pfr/pfrobjs.c
    freetype2/src/pfr/pfrdrivr.c
    freetype2/src/pfr/pfrsbit.c
    freetype2/src/pfr/pfrgload.c
    freetype2/src/psaux/afmparse.c
    freetype2/src/psaux/psobjs.c
    freetype2/src/psaux/psaux.c
    freetype2/src/psaux/t1cmap.c
    freetype2/src/psaux/psauxmod.c
    freetype2/src/psaux/t1decode.c
    freetype2/src/psaux/psconv.c
    freetype2/src/pshinter/pshinter.c
    freetype2/src/psnames/psnames.c
    freetype2/src/raster/raster.c
    freetype2/src/sfnt/sfnt.c
    freetype2/src/smooth/smooth.c
    freetype2/src/truetype/truetype.c
    freetype2/src/type1/t1afm.c
    freetype2/src/type1/t1objs.c
    freetype2/src/type1/t1driver.c
    freetype2/src/type1/t1parse.c
    freetype2/src/type1/t1gload.c
    freetype2/src/type1/type1.c
    freetype2/src/type1/t1load.c
    freetype2/src/type42/t42drivr.c
    freetype2/src/type42/t42parse.c
    freetype2/src/type42/t42objs.c
    freetype2/src/type42/type42.c
    freetype2/src/winfonts/winfnt.c
)

target_include_directories(freetype
    PUBLIC freetype2/include
    PRIVATE freetype2/include/freetype/config
    PRIVATE freetype2/internal
)

target_compile_definitions(freetype
    PRIVATE -DFT_CONFIG_MODULES_H=<ftmodule.h>
    PRIVATE -DFT2_BUILD_LIBRARY=1
)
