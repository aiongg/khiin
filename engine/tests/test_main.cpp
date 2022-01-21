#pragma once

#ifdef _WIN32
#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#endif

#define BOOST_TEST_MODULE TaiKeyTest
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN
#include <boost/test/debug.hpp>
#include <boost/test/unit_test.hpp>

int main(int argc, char *argv[], char *envp[]) {
    return boost::unit_test::unit_test_main(&init_unit_test, argc, argv);
}

struct GlobalFixture {
    GlobalFixture() {}
    ~GlobalFixture() {}
};

BOOST_GLOBAL_FIXTURE(GlobalFixture);
