# TODO: src/ should have its own cmakelists in the future

cmake_minimum_required(VERSION 3.10)

project(GEDCOM_Parser C)

FILE(GLOB CSource ${PROJECT_SOURCE_DIR}/main.c ${PROJECT_SOURCE_DIR}/src/*.c ${PROJECT_SOURCE_DIR}/src/tags/*.c)

include_directories(GEDCOM_Parser ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src/)
add_executable(GEDCOM_Parser ${CSource})
