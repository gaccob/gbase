# compatible for travis CI
cmake_minimum_required(VERSION 2.8.7)
set(TEST_NAME "unitest")
project("${TEST_NAME}")

aux_source_directory(. TEST_SOURCES)
aux_source_directory(./unit TEST_SOURCES)

# set openssl directories
find_package(OpenSSL)
if (OPENSSL_FOUND)
    message("openssl include: ${OPENSSL_INCLUDE_DIR}")
    include_directories("${OPENSSL_INCLUDE_DIR}")
else ()
    message("openssl include: ${OPENSSL_ROOT_DIR}/include")
    include_directories("${OPENSSL_ROOT_DIR}/include")
endif (OPENSSL_FOUND)

# link
add_executable(${TEST_NAME} ${TEST_SOURCES})
target_link_libraries(${TEST_NAME} ${GBASE_LIB} ${GBASE_LIB_LINK} ${OPENSSL_LIBRARIES} readline)

# copy test source file
set(TEST_CONFIG_FILE
    "assets/dirty_cfg_words"
    "assets/ws_client.html"
)
foreach(f ${TEST_CONFIG_FILE})
    configure_file(${f} .)
endforeach()

