cmake_minimum_required(VERSION 2.8.10)
set(TEST_NAME "ssl_test")
project("${TEST_NAME}")
aux_source_directory(. TEST_SOURCES)

# set openssl default installer directory
set (OPENSSL_ROOT "/usr/local/ssl")
include_directories("${OPENSSL_ROOT}/include")
link_directories("${OPENSSL_ROOT}/lib")

add_executable("${TEST_NAME}" "${TEST_SOURCES}")
target_link_libraries("${TEST_NAME}" ${GBASE_LIB} ${GBASE_LIB_LINK} crypto)
