cmake_minimum_required(VERSION 3.0)

project(harfbuzz)

include(CheckIncludeFile)
include(CheckSymbolExists)

add_library(harfbuzz STATIC
    harfbuzz/src/hb-aat-layout.cc
    harfbuzz/src/hb-blob.cc
    harfbuzz/src/hb-buffer-serialize.cc
    harfbuzz/src/hb-buffer.cc
    harfbuzz/src/hb-common.cc
    # harfbuzz/src/hb-coretext.cc
    # harfbuzz/src/hb-directwrite.cc
    harfbuzz/src/hb-face.cc
    # harfbuzz/src/hb-fallback-shape.cc
    harfbuzz/src/hb-font.cc
    harfbuzz/src/hb-ft.cc
    # harfbuzz/src/hb-glib.cc
    # harfbuzz/src/hb-gobject-structs.cc
    # harfbuzz/src/hb-graphite2.cc
    # harfbuzz/src/hb-icu.cc
    harfbuzz/src/hb-ot-face.cc
    harfbuzz/src/hb-ot-font.cc
    harfbuzz/src/hb-ot-layout.cc
    harfbuzz/src/hb-ot-map.cc
    harfbuzz/src/hb-ot-shape-complex-arabic.cc
    harfbuzz/src/hb-ot-shape-complex-default.cc
    harfbuzz/src/hb-ot-shape-complex-hangul.cc
    harfbuzz/src/hb-ot-shape-complex-hebrew.cc
    harfbuzz/src/hb-ot-shape-complex-indic-table.cc
    harfbuzz/src/hb-ot-shape-complex-indic.cc
    harfbuzz/src/hb-ot-shape-complex-khmer.cc
    harfbuzz/src/hb-ot-shape-complex-myanmar.cc
    harfbuzz/src/hb-ot-shape-complex-thai.cc
    harfbuzz/src/hb-ot-shape-complex-use-table.cc
    harfbuzz/src/hb-ot-shape-complex-use.cc
    harfbuzz/src/hb-ot-shape-fallback.cc
    harfbuzz/src/hb-ot-shape-normalize.cc
    harfbuzz/src/hb-ot-shape.cc
    harfbuzz/src/hb-ot-tag.cc
    harfbuzz/src/hb-ot-var.cc
    harfbuzz/src/hb-set.cc
    harfbuzz/src/hb-shape-plan.cc
    harfbuzz/src/hb-shape.cc
    harfbuzz/src/hb-shaper.cc
    harfbuzz/src/hb-static.cc
    harfbuzz/src/hb-ucdn.cc
    harfbuzz/src/hb-ucdn/ucdn.c
    harfbuzz/src/hb-unicode.cc
    # harfbuzz/src/hb-uniscribe.cc
    # harfbuzz/src/hb-warning.cc

    harfbuzz/src/hb-buffer-deserialize-json.rl
    harfbuzz/src/hb-buffer-deserialize-text.rl
    harfbuzz/src/hb-ot-shape-complex-indic-machine.rl
    harfbuzz/src/hb-ot-shape-complex-khmer-machine.rl
    harfbuzz/src/hb-ot-shape-complex-myanmar-machine.rl
    harfbuzz/src/hb-ot-shape-complex-use-machine.rl
)

set_target_properties(harfbuzz PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(memory.h HAVE_MEMORY_H)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(sys/mman.h HAVE_SYS_MMAN_H)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(unistd.h HAVE_UNISTD_H)

include(CheckSymbolExists)
check_symbol_exists(atexit stdlib.h HAVE_ATEXIT)
check_symbol_exists(getpagesize unistd.h HAVE_GETPAGESIZE)
check_symbol_exists(mmap sys/mman.h HAVE_MMAP)
check_symbol_exists(mprotect sys/mman.h HAVE_MPROTECT)
check_symbol_exists(round math.h HAVE_ROUND)
check_symbol_exists(sysconf unistd.h HAVE_SYSCONF)

target_include_directories(harfbuzz
    PUBLIC harfbuzz/src
    PRIVATE harfbuzz/src/hb-ucdn
    PRIVATE autoconf_generated
)

target_link_libraries(harfbuzz freetype)

target_compile_definitions(harfbuzz
    PRIVATE HAVE_FREETYPE HAVE_OT HAVE_UCDN
        HAVE_FT_GET_VAR_BLEND_COORDINATES
        HAVE_INTEL_ATOMIC_PRIMITIVES
        $<IF:${HAVE_ATEXIT},HAVE_ATEXIT,>
        $<IF:${HAVE_GETPAGESIZE},HAVE_GETPAGESIZE,>
        $<IF:${HAVE_MMAP},HAVE_MMAP,>
        $<IF:${HAVE_MPROTECT},HAVE_MPROTECT,>
        $<IF:${HAVE_ROUND},HAVE_ROUND,>
        $<IF:${HAVE_SYSCONF},HAVE_SYSCONF,>
        $<IF:${HAVE_SYS_MMAN_H},HAVE_SYS_MMAN_H,>
        $<IF:${HAVE_SYS_STAT_H},HAVE_SYS_STAT_H,>
        $<IF:${HAVE_SYS_TYPES_H},HAVE_SYS_TYPES_H,>
        $<IF:${HAVE_UNISTD_H},HAVE_UNISTD_H,>
)
