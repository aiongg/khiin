include_directories(${TaiKey_SOURCE_DIR}/libtaikey)

add_executable(libtaikey_test "test_main.cpp" "test_engine.cpp" "test_trie.cpp"
    "test_buffer.cpp" "test_lomaji.cpp"  "test_splitter.cpp" "test_db.cpp" "test_candidates.cpp")

target_link_libraries(libtaikey_test ${Boost_LIBRARIES} SQLiteCpp libtaikey)

add_test(NAME libtaikey_test COMMAND libtaikey_test)

file(GLOB resources "${TaiKey_SOURCE_DIR}/../resources/*")
message("Resource file list: ${resources}")
foreach(res ${resources})
    add_custom_command(
        TARGET libtaikey_test
        POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E copy
            ${res}
            ${CMAKE_CURRENT_BINARY_DIR}
    )
endforeach()
