# CMakeList.txt : CMake project for libtaikey, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_library (khiin
    "${PROTOBUF_DIR}/config.pb.h"
    "${PROTOBUF_DIR}/config.pb.cc"
    "${PROTOBUF_DIR}/command.pb.h"
    "${PROTOBUF_DIR}/command.pb.cc"
    "${PROTOBUF_DIR}/proto.h"
    "CandidateFinder.h"
    "CandidateFinder.cpp"
    "BufferElement.h"
    "BufferElement.cpp"
    "BufferMgr.h"
    "BufferMgr.cpp"
    "TaiText.h"
    "TaiText.cpp"
    "Lomaji.h"
    "Lomaji.cpp"
    "common.h"
    "Database.h"
    "Database.cpp"
    "Dictionary.h"
    "Dictionary.cpp"
    "errors.h"
    "Engine.h"
    "Engine.cpp"
    "KeyConfig.h"
    "KeyConfig.cpp"
    "Segmenter.h"
    "Segmenter.cpp"
    "SQL.h"
    "Splitter.h"
    "Splitter.cpp"
    "Syllable.h"
    "Syllable.cpp"
    "SyllableParser.h"
    "SyllableParser.cpp"
    "Trie.h"
    "Trie.cpp"
    "unicode_utils.h"
    "unicode_utils.cpp"
    "utils.h"
 "Models.h"   "Config.h" "KhinHandler.h" "KhinHandler.cpp"   "Buffer.h" "Buffer.cpp" "utils.cpp"    "Config.cpp")

# Debug
target_link_options(khiin PRIVATE "/MAP")
