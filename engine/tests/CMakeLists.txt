include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test_khiin_engine
    "TestMain.cpp"
    "EngineTest.cpp"
    "TrieTest.cpp"
    "test_buffer.cpp"
    "LomajiTest.cpp"
    "DatabaseTest.cpp"
    "SyllableTest.cpp"
    "KeyConfigTest.cpp"
    "SyllableParserTest.cpp"
    "DictionaryTest.cpp"
    "BufferMgrTest.cpp"
    "SegmenterTest.cpp"
    "SplitterTest.cpp"
    "TestEnv.h"
    "UnicodeTest.cpp"
    "UtilsTest.cpp"
    "BufferMgrTestBase.h"
    "BufferMgrTestBase.cpp"
    "BasicModeTest.cpp"
    "NgramTest.cpp"
    "UserDictionaryTest.cpp"
)

target_link_libraries(test_khiin_engine khiin SQLiteCpp protobuf::libprotobuf-lite GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)

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
