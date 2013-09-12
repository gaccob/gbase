cmake_minimum_required(VERSION 2.8.10)
set(TEST_NAME "tcp_test")
project("${TEST_NAME}")

add_executable(tcp_client "client.c")
add_executable(tcp_server "server.c")
target_link_libraries(tcp_client ${GBASE_LIB} ${GBASE_LIB_LINK})
target_link_libraries(tcp_server ${GBASE_LIB} ${GBASE_LIB_LINK})

