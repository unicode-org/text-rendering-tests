cmake_minimum_required(VERSION 3.0)

project(sheenbidi)

add_library(sheenbidi STATIC
    sheenbidi/Source/SheenBidi.c
)

target_include_directories(sheenbidi
    PUBLIC sheenbidi/Headers
)

target_compile_definitions(sheenbidi
    PRIVATE -DSB_CONFIG_UNITY=1
)
