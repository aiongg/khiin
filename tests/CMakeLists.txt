include_directories(${TaiKey_SOURCE_DIR}/libtaikey)

add_executable(libtaikey_test "test_main.cpp" "test_engine.cpp" "test_trie.cpp" "test_buffer.cpp")
target_link_libraries(libtaikey_test ${Boost_LIBRARIES} libtaikey)

add_test(NAME libtaikey_test COMMAND libtaikey_test)
