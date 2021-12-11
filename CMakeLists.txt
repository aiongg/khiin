# CMakeList.txt : CMake project for libtaikey, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_library (libtaikey "libtaikey.h" "libtaikey.cpp" "keys.h" "trie.h" "trie.cpp")

# TODO: Add tests and install targets if needed.

# Debug
target_link_options(libtaikey PRIVATE "/MAP")
