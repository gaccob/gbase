cmake_minimum_required(VERSION 2.8.10)
set(TEST_NAME "test_shm")
project("${TEST_NAME}")

add_executable(shm_send "test_send.c")
add_executable(shm_recv "test_recv.c")
target_link_libraries(shm_send ${GBASE_LIB} ${GBASE_LIB_LINK})
target_link_libraries(shm_recv ${GBASE_LIB} ${GBASE_LIB_LINK})

