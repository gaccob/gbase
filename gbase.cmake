include("${CMAKE_SOURCE_DIR}/common.cmake")

# 版本号
set(MAJOR_VERSION 0)
set(MINOR_VERSION 1)

# 包含头文件
include_directories("${CMAKE_SOURCE_DIR}")

# 源码目录
set(GBASE_DIR_CORE "${CMAKE_SOURCE_DIR}/core")
set(GBASE_DIR_BASE "${CMAKE_SOURCE_DIR}/base")
set(GBASE_DIR_UTIL "${CMAKE_SOURCE_DIR}/util")
set(GBASE_DIR_TEST "${CMAKE_SOURCE_DIR}/test")
set(GBASE_DIR_NET "${CMAKE_SOURCE_DIR}/net")
set(GBASE_DIR_MM "${CMAKE_SOURCE_DIR}/mm")
set(GBASE_DIR_LOGIC "${CMAKE_SOURCE_DIR}/logic")

# 发布目录
string(TIMESTAMP TS "%y%m%d")
set(GBASE_DIR_RELEASE "${CMAKE_SOURCE_DIR}/release-${MAJOR_VERSION}.${MINOR_VERSION}.${TS}")

# 编译目标
set(GBASE_LIB gbase)

# 链接选项
set (GBASE_LIB_LINK ${COMMON_LINK_LIB})

# 编译选项
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wno-error=unused-but-set-variable")

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

# 编译test
add_subdirectory(${GBASE_DIR_TEST})

# 安装到发布目录
install(
    DIRECTORY ${GBASE_DIR_CORE} ${GBASE_DIR_BASE} ${GBASE_DIR_UTIL} ${GBASE_DIR_NET} ${GBASE_DIR_MM} ${GBASE_DIR_LOGIC}
    DESTINATION "${GBASE_DIR_RELEASE}/include"
    USE_SOURCE_PERMISSIONS
    FILES_MATCHING PATTERN "*.h" 
)
install(
    TARGETS ${GBASE_LIB}
    DESTINATION "${GBASE_DIR_RELEASE}/lib"
)

