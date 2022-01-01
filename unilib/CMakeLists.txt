add_library(unilib STATIC
    "unicode.h"
    "unicode.cpp"
    "uninorms.h"
    "uninorms.cpp"
    "unistrip.h"
    "unistrip.cpp"
    "utf8.h"
    "utf8.cpp"
    "utf16.h"
    "utf16.cpp"
    "version.h"
    "version.cpp"
)

if (MSVC)
    target_compile_definitions(
        unilib PRIVATE -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
    )
    target_compile_options(
        unilib PRIVATE /wd4018 /wd4244
    )
endif()
