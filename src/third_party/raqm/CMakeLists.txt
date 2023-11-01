cmake_minimum_required(VERSION 3.0)

project(raqm)

include(CheckIncludeFile)
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(memory.h HAVE_MEMORY_H)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(unistd.h HAVE_UNISTD_H)

add_library(raqm STATIC
    libraqm/src/raqm.c
)

target_compile_definitions(raqm
    PRIVATE
        HAVE_HB_FT_FONT_CREATE_REFERENCED
        HAVE_DECL_HB_BUFFER_FLAG_REMOVE_DEFAULT_IGNORABLES
        HAVE_HB_BUFFER_SET_INVISIBLE_GLYPH
        HAVE_HB_FT_FONT_SET_LOAD_FLAGS
        $<IF:$<BOOL:${HAVE_INTTYPES_H}>,HAVE_INTTYPES_H,>
        $<IF:$<BOOL:${HAVE_MEMORY_H}>,HAVE_MEMORY_H,>
        $<IF:$<BOOL:${HAVE_STDINT_H}>,HAVE_STDINT_H,>
        $<IF:$<BOOL:${HAVE_STDLIB_H}>,HAVE_STDLIB_H,>
        $<IF:$<BOOL:${HAVE_STRING_H}>,HAVE_STRING_H,>
        $<IF:$<BOOL:${HAVE_STRINGS_H}>,HAVE_STRINGS_H,>
        $<IF:$<BOOL:${HAVE_SYS_STAT_H}>,HAVE_SYS_STAT_H,>
        $<IF:$<BOOL:${HAVE_SYS_TYPES_H}>,HAVE_SYS_TYPES_H,>
        $<IF:$<BOOL:${HAVE_UNISTD_H}>,HAVE_UNISTD_H,>
)

target_include_directories(raqm SYSTEM
    PUBLIC libraqm/src
    PUBLIC generated
)

target_link_libraries(raqm fribidi harfbuzz)
