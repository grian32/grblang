cmake_minimum_required(VERSION 4.0)
project(grblang C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -g")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

add_library(grblang_lib lexer.c
        parser.c
        bytecode_emit.c
        lexer.h
        parser.h
        bytecode_emit.h
        util.h
        stack.c
        stack.h
        vm.c
        vm.h
        resolver.c
        resolver.h
        type_checker.c
        type_checker.h)

target_include_directories(grblang_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(grblang main.c)
target_link_libraries(grblang PRIVATE grblang_lib)
