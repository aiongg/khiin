# CMakeList.txt : CMake project for libtaikey, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_library (khiin_engine
    "buffer.h"
    "buffer.cpp"
    "buffer_manager.h"
    "buffer_manager.cpp"
    "candidates.h"
    "candidates.cpp"
    "common.h"
    "config.h"
    "config.cpp"
    "cursor.cpp"
    "db.h"
    "db.cpp"
    "errors.h"
    "keys.h"
    "engine.h"
    "engine.cpp"
    "lomaji.h"
    "lomaji.cpp"
    "sql.h"
    "splitter.h"
    "splitter.cpp"
    "trie.h"
    "trie.cpp" "utils.h"
    "${PROTOBUF_DIR}/messages.pb.h"
    "${PROTOBUF_DIR}/messages.pb.cc"
 "messages.h")

# Debug
target_link_options(khiin_engine PRIVATE "/MAP")
