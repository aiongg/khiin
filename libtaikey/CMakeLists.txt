# CMakeList.txt : CMake project for libtaikey, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_library (libtaikey "libtaikey.h" "libtaikey.cpp"
    "keys.h" "trie.h" "trie.cpp" "errors.h" "buffer.h"
    "buffer.cpp" "common.h" "tk_locale.h" "lomaji.h" "lomaji.cpp"
    "syl_splitter.h" "syl_splitter.cpp" "db.h" "db.cpp" "candidates.h" "candidates.cpp" "sql.h" "engine.h" "engine.cpp" "config.h" "config.cpp")

set_property(TARGET libtaikey PROPERTY CXX_STANDARD 17)

# TODO: Add tests and install targets if needed.
target_include_directories(libtaikey PUBLIC "${CMAKE_SOURCE_DIR}/SQLiteCpp/include")

# Debug
target_link_options(libtaikey PRIVATE "/MAP")
