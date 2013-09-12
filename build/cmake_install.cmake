# Install script for directory: /Users/zhangweiye/Documents/data/code/gaccob/gbase

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Debug")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/Users/zhangweiye/Documents/data/code/gaccob/gbase/build/test/atom_test/cmake_install.cmake")
  INCLUDE("/Users/zhangweiye/Documents/data/code/gaccob/gbase/build/test/bevtree_test/cmake_install.cmake")
  INCLUDE("/Users/zhangweiye/Documents/data/code/gaccob/gbase/build/test/bitset_test/cmake_install.cmake")
  INCLUDE("/Users/zhangweiye/Documents/data/code/gaccob/gbase/build/test/heap_test/cmake_install.cmake")
  INCLUDE("/Users/zhangweiye/Documents/data/code/gaccob/gbase/build/test/json_test/cmake_install.cmake")
  INCLUDE("/Users/zhangweiye/Documents/data/code/gaccob/gbase/build/test/rbtree_test/cmake_install.cmake")
  INCLUDE("/Users/zhangweiye/Documents/data/code/gaccob/gbase/build/test/rbuffer_test/cmake_install.cmake")
  INCLUDE("/Users/zhangweiye/Documents/data/code/gaccob/gbase/build/test/slist_test/cmake_install.cmake")
  INCLUDE("/Users/zhangweiye/Documents/data/code/gaccob/gbase/build/test/spin_test/cmake_install.cmake")
  INCLUDE("/Users/zhangweiye/Documents/data/code/gaccob/gbase/build/test/tcp_test/cmake_install.cmake")
  INCLUDE("/Users/zhangweiye/Documents/data/code/gaccob/gbase/build/test/test/cmake_install.cmake")
  INCLUDE("/Users/zhangweiye/Documents/data/code/gaccob/gbase/build/test/thread_test/cmake_install.cmake")
  INCLUDE("/Users/zhangweiye/Documents/data/code/gaccob/gbase/build/test/timer_test/cmake_install.cmake")
  INCLUDE("/Users/zhangweiye/Documents/data/code/gaccob/gbase/build/test/wsconn_test/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

IF(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
ELSE(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
ENDIF(CMAKE_INSTALL_COMPONENT)

FILE(WRITE "/Users/zhangweiye/Documents/data/code/gaccob/gbase/build/${CMAKE_INSTALL_MANIFEST}" "")
FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})
  FILE(APPEND "/Users/zhangweiye/Documents/data/code/gaccob/gbase/build/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
ENDFOREACH(file)
