include("${CMAKE_SOURCE_DIR}/common.cmake")

# 包含头文件
include_directories("${CMAKE_SOURCE_DIR}")
COMMON_INCLUDE("${CMAKE_SOURCE_DIR}")

set(GBASE_DIR_CORE "${CMAKE_SOURCE_DIR}/core")
set(GBASE_DIR_DS "${CMAKE_SOURCE_DIR}/ds")
set(GBASE_DIR_NET "${CMAKE_SOURCE_DIR}/net")
set(GBASE_DIR_TEST "${CMAKE_SOURCE_DIR}/test")

set(GBASE_LIB gbase)

file(GLOB_RECURSE GBASE_SOURCE
    "${GBASE_DIR_CORE}/*.h" "${GBASE_DIR_CORE}/*.c"
    "${GBASE_DIR_DS}/*.h" "${GBASE_DIR_DS}/*.c"
    "${GBASE_DIR_NET}/*.h" "${GBASE_DIR_NET}/*.c")

add_library(${GBASE_LIB} ${GBASE_SOURCE})
