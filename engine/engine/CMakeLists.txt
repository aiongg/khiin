# CMakeList.txt : CMake project for libtaikey, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_library (khiin
    "SynchronizedBuffer.h"      # delete
    "SynchronizedBuffer.cpp"    # delete
    "BufferManager.h"           # delete
    "BufferManager.cpp"         # delete
    "Config.h"                  # delete
    "Config.cpp"                # delete
    "Caret.cpp"                 # delete
    "Lomaji.h"                  # delete
    "Lomaji.cpp"                # delete
    "ParallelIterators.h"       # delete
              # delete
            # delete
    "${PROTOBUF_DIR}/messages.pb.h"
    "${PROTOBUF_DIR}/messages.pb.cc"
    "CandidateFinder.h"
    "CandidateFinder.cpp"
    "BufferElement.h"
    "BufferElement.cpp"
    "BufferMgr.h"
    "BufferMgr.cpp"
    "TaiText.h"
    "TaiText.cpp"
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
    "messages.h"
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
 "Models.h")

# Debug
target_link_options(khiin PRIVATE "/MAP")
