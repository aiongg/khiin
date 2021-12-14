#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "buffer.h"

using namespace TaiKey;

BOOST_AUTO_TEST_SUITE(BufferTest);

BOOST_AUTO_TEST_CASE(Trivial) { BOOST_CHECK(true); }

BOOST_AUTO_TEST_CASE(Buffer_Create) {
    Buffer buf;
    BOOST_CHECK_EQUAL(buf.getDisplayBuffer(), "");
    BOOST_CHECK_EQUAL(buf.getCursor(), 0);
}

BOOST_AUTO_TEST_CASE(Buffer_Input) {
    Buffer *buf = new Buffer();
    buf->setToneKeys(ToneKeys::Telex);

    buf->insert('a');
    BOOST_CHECK_EQUAL(buf->getDisplayBuffer(), u8"a");
    BOOST_CHECK_EQUAL(buf->getCursor(), 1);

    buf->insert('s');
    BOOST_CHECK_EQUAL(buf->getDisplayBuffer(), u8"á");
    BOOST_CHECK_EQUAL(buf->getCursor(), 1);
}

BOOST_AUTO_TEST_SUITE_END();
