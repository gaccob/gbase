cmake_minimum_required(VERSION 2.8.10)
set(TEST_NAME "test")
project("${TEST_NAME}")
aux_source_directory(. TEST_SOURCES)
add_executable("${TEST_NAME}" "${TEST_SOURCES}")
target_link_libraries("${TEST_NAME}" ${GBASE_LIB} ${GBASE_LIB_LINK})

# copy test source file
set(TEST_CONFIG_FILE
    "dirty_cfg_words"
    "dirty_test_words"
    "json_test_file"
)
foreach(f ${TEST_CONFIG_FILE})
    configure_file(${f} .)
endforeach()
