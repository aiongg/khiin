include_directories(${PROJECT_SOURCE_DIR}/engine)

add_executable(test_khiin_engine "test_main.cpp" "test_engine.cpp" "test_trie.cpp"
    "test_buffer.cpp" "test_lomaji.cpp"  "test_splitter.cpp" "test_db.cpp" "test_candidates.cpp")

target_link_libraries(test_khiin_engine ${Boost_LIBRARIES} SQLiteCpp khiin_engine)

add_test(NAME test_khiin_engine COMMAND test_khiin_engine)

file(GLOB resources "${PROJECT_SOURCE_DIR}/../resources/*")
message("Resource file list: ${resources}")
foreach(res ${resources})
    add_custom_command(
        TARGET test_khiin_engine
        POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E copy
            ${res}
            ${CMAKE_CURRENT_BINARY_DIR}
    )
endforeach()
