# CMakeList.txt : CMake project for libtaikey, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_library (khiin_engine
    "SynchronizedBuffer.h"
    "SynchronizedBuffer.cpp"
    "BufferManager.h"
    "BufferManager.cpp"
    "CandidateFinder.h"
    "CandidateFinder.cpp"
    "common.h"
    "Config.h"
    "Config.cpp"
    "Caret.cpp"
    "Database.h"
    "Database.cpp"
    "errors.h"
    "Engine.h"
    "Engine.cpp"
    "Lomaji.h"
    "Lomaji.cpp"
    "SQL.h"
    "Splitter.h"
    "Splitter.cpp"
    "Trie.h"
    "Trie.cpp" "utils.h"
    "messages.h"
    "${PROTOBUF_DIR}/messages.pb.h"
    "${PROTOBUF_DIR}/messages.pb.cc"
   "ParallelIterators.h")

# Debug
target_link_options(khiin_engine PRIVATE "/MAP")
