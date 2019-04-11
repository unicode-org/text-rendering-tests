cmake_minimum_required(VERSION 3.0)

project(sheenfigure)

add_library(sheenfigure STATIC
    sheenfigure/Source/SheenFigure.c
)

target_include_directories(sheenfigure
    PUBLIC sheenfigure/Headers
)

target_compile_definitions(sheenfigure
    PRIVATE -DSF_CONFIG_UNITY=1
)

target_link_libraries(sheenfigure sheenbidi)
