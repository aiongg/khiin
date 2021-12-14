// libtaikey_test.h : Header file for your target.

#pragma once

#ifdef _WIN32
#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#endif

#define BOOST_TEST_MODULE TaiKeyTest
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>

#include "tk_locale.h"

int main(int argc, char *argv[], char *envp[]) {

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    char *pcDynamicHeapStart = new char[17u];
    strcpy_s(pcDynamicHeapStart, 17u, "DynamicHeapStart");

    TaiKey::init_locale();
    TaiKey::init_locale();
    return boost::unit_test::unit_test_main(&init_unit_test, argc, argv);
}
