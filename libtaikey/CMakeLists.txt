# CMakeList.txt : CMake project for libtaikey, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_library (libtaikey "libtaikey.h" "libtaikey.cpp"
    "keys.h" "trie.h" "trie.cpp" "errors.h" "buffer.h"
    "buffer.cpp" "common.h" "tk_locale.h" "lomaji.h" "lomaji.cpp"
    "syl_splitter.h" "syl_splitter.cpp" "db.h" "db.cpp")

# TODO: Add tests and install targets if needed.

# Debug
target_link_options(libtaikey PRIVATE "/MAP")
