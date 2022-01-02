# CMakeList.txt : CMake project for libtaikey, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_library (libtaikey
    "buffer_manager.h"
    "buffer_manager.cpp"
    "candidates.h"
    "candidates.cpp"
    "common.h"
    "config.h"
    "config.cpp"
    "db.h"
    "db.cpp"
    "engine.h"
    "engine.cpp"
    "errors.h"
    "keys.h"
    "libtaikey.h"
    "libtaikey.cpp"
    "lomaji.h"
    "lomaji.cpp"
    "sql.h"
    "syl_splitter.h"
    "syl_splitter.cpp"
    "trie.h"
    "trie.cpp"
 "buffer.h" "buffer.cpp")

# Debug
target_link_options(libtaikey PRIVATE "/MAP")
