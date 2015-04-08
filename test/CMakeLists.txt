# compatible for travis CI
cmake_minimum_required(VERSION 2.8.7)
set(TEST_NAME "test")
project("${TEST_NAME}")

aux_source_directory(. TEST_SOURCES)
aux_source_directory(./unit TEST_SOURCES)

# set openssl default installer directory
set (OPENSSL_ROOT "/usr/local/ssl")
include_directories("${OPENSSL_ROOT}/include")
link_directories("${OPENSSL_ROOT}/lib")

# link
add_executable(${TEST_NAME} ${TEST_SOURCES})
target_link_libraries(${TEST_NAME} ${GBASE_LIB} ${GBASE_LIB_LINK} crypto readline)

# copy test source file
set(TEST_CONFIG_FILE
    "assets/dirty_cfg_words"
    "assets/bevtree.jpg"
    "assets/bevtree.json"
    "assets/ws_client.html"
)
foreach(f ${TEST_CONFIG_FILE})
    configure_file(${f} .)
endforeach()

