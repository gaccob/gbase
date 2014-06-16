include("${CMAKE_SOURCE_DIR}/common.cmake")

# 包含头文件
include_directories("${CMAKE_SOURCE_DIR}")

set(GBASE_DIR_CORE "${CMAKE_SOURCE_DIR}/core")
set(GBASE_DIR_BASE "${CMAKE_SOURCE_DIR}/base")
set(GBASE_DIR_UTIL "${CMAKE_SOURCE_DIR}/util")
set(GBASE_DIR_TEST "${CMAKE_SOURCE_DIR}/test")
set(GBASE_DIR_NET "${CMAKE_SOURCE_DIR}/net")
set(GBASE_DIR_MM "${CMAKE_SOURCE_DIR}/mm")
set(GBASE_DIR_LOGIC "${CMAKE_SOURCE_DIR}/logic")

set(GBASE_LIB gbase)

# 链接选项
set (GBASE_LIB_LINK ${COMMON_LINK_LIB})

# 告警设置(for default C debug)
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wno-error=unused-but-set-variable")
message(STATUS "cmake C debug option: ${CMAKE_C_FLAGS_DEBUG}")

# 编译lib的源文件
aux_source_directory(${GBASE_DIR_CORE} GBASE_SOURCE)
aux_source_directory(${GBASE_DIR_BASE} GBASE_SOURCE)
aux_source_directory(${GBASE_DIR_NET} GBASE_SOURCE)
aux_source_directory(${GBASE_DIR_UTIL} GBASE_SOURCE)
aux_source_directory(${GBASE_DIR_MM} GBASE_SOURCE)
aux_source_directory(${GBASE_DIR_LOGIC} GBASE_SOURCE)

foreach(GBASE_SOURCE_FILE ${GBASE_SOURCE})
    CommonEcho(COLOR CYAN "===> source: ${GBASE_SOURCE_FILE}")
endforeach()

# 编译lib
add_library(${GBASE_LIB} ${GBASE_SOURCE})

# test
add_subdirectory(${GBASE_DIR_TEST})

