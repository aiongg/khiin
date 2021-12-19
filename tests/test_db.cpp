#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "db.h"

using namespace TaiKey;

BOOST_AUTO_TEST_SUITE(Database)

BOOST_AUTO_TEST_CASE(test_db_connection) {
    TKDB db("taikey.db");
}

BOOST_AUTO_TEST_SUITE_END()