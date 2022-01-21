include_directories(${PROJECT_SOURCE_DIR}/taikey)

add_executable(taikey_test "test_main.cpp" "test_engine.cpp" "test_trie.cpp"
    "test_buffer.cpp" "test_lomaji.cpp"  "test_splitter.cpp" "test_db.cpp" "test_candidates.cpp")

target_link_libraries(taikey_test ${Boost_LIBRARIES} SQLiteCpp taikey)

add_test(NAME taikey_test COMMAND taikey_test)

file(GLOB resources "${PROJECT_SOURCE_DIR}/../resources/*")
message("Resource file list: ${resources}")
foreach(res ${resources})
    add_custom_command(
        TARGET taikey_test
        POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E copy
            ${res}
            ${CMAKE_CURRENT_BINARY_DIR}
    )
endforeach()
