include("${CMAKE_SOURCE_DIR}/common.cmake")

# 包含头文件
include_directories("${CMAKE_SOURCE_DIR}")

set(GBASE_DIR_CORE "${CMAKE_SOURCE_DIR}/core")
set(GBASE_DIR_BASE "${CMAKE_SOURCE_DIR}/base")
set(GBASE_DIR_UTIL "${CMAKE_SOURCE_DIR}/util")
set(GBASE_DIR_TEST "${CMAKE_SOURCE_DIR}/test")
set(GBASE_DIR_NET "${CMAKE_SOURCE_DIR}/net")
set(GBASE_DIR_MM "${CMAKE_SOURCE_DIR}/mm")

set(GBASE_DIR_LOGIC_DIRTY "${CMAKE_SOURCE_DIR}/logic/dirty")
set(GBASE_DIR_LOGIC_BUS "${CMAKE_SOURCE_DIR}/logic/bus")

set(GBASE_LIB gbase)

# 链接选项
set (GBASE_LIB_LINK ${COMMON_LINK_LIB})

# 编译lib的源文件
aux_source_directory(${GBASE_DIR_CORE} GBASE_SOURCE)
aux_source_directory(${GBASE_DIR_BASE} GBASE_SOURCE)
aux_source_directory(${GBASE_DIR_NET} GBASE_SOURCE)
aux_source_directory(${GBASE_DIR_UTIL} GBASE_SOURCE)
aux_source_directory(${GBASE_DIR_MM} GBASE_SOURCE)
aux_source_directory(${GBASE_DIR_LOGIC_DIRTY} GBASE_SOURCE)
aux_source_directory(${GBASE_DIR_LOGIC_BUS} GBASE_SOURCE)

foreach(GBASE_SOURCE_FILE ${GBASE_SOURCE})
    CommonEcho(COLOR CYAN "===> source: ${GBASE_SOURCE_FILE}")
endforeach()

# 编译lib
add_library(${GBASE_LIB} ${GBASE_SOURCE})

# 递归增加test
file(GLOB GBASE_TEST_DIRS ${GBASE_DIR_TEST}/*test*)
foreach(GBASE_TEST_DIR ${GBASE_TEST_DIRS})
    CommonEcho(COLOR CYAN "===> directory: ${GBASE_TEST_DIR}")
    add_subdirectory(${GBASE_TEST_DIR})
endforeach()
