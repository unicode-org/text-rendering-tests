cmake_minimum_required(VERSION 3.0)

project(fonttest)

add_executable(fonttest
    main.cpp
    font_engine.cpp
    freestack_engine.cpp
    freestack_font.cpp
    freestack_line.cpp
    freestack_path.cpp
    test_harness.cpp
    $<IF:${APPLE},coretext_engine.mm,>
    $<IF:${APPLE},coretext_font.mm,>
    $<IF:${APPLE},coretext_line.mm,>
    $<IF:${APPLE},coretext_path.mm,>
)

set_target_properties(fonttest PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(fonttest
    PRIVATE ..
)

set(compile_definitions)
if(APPLE)
    list(APPEND compile_definitions HAVE_CORETEXT)
    include(CheckCSourceCompiles)
    check_c_source_compiles("
        #include <CoreText/CoreText.h>
        int main(int a, char** b) { return (int) kCTVersionNumber10_14; }
        " HAVE_CORETEXT_10_14)
    if(DEFINED HAVE_CORETEXT_10_14 AND HAVE_CORETEXT_10_14)
        list(APPEND compile_definitions HAVE_CORETEXT_10_14)
    endif()
endif()

target_compile_definitions(fonttest
    PRIVATE ${compile_definitions}
)

if(APPLE)
  find_library(Foundation Foundation)
  find_library(CoreGraphics CoreGraphics)
  find_library(CoreText CoreText)
endif(APPLE)

target_link_libraries(fonttest
    freetype harfbuzz raqm sheenbidi sheenfigure
    $<IF:${APPLE},${Foundation},>
    $<IF:${APPLE},${CoreGraphics},>
    $<IF:${APPLE},${CoreText},>
)
