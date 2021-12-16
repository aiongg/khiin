#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "buffer.h"

using namespace TaiKey;

BOOST_AUTO_TEST_SUITE(BufferTest);

BOOST_AUTO_TEST_CASE(Trivial) { BOOST_TEST(true); }

BOOST_AUTO_TEST_CASE(Buffer_Create) {
    Buffer *buf = new Buffer();
    BOOST_TEST(buf->getDisplayBuffer() == "");
    BOOST_TEST(buf->getCursor() == 0);

    delete buf;
}

BOOST_AUTO_TEST_CASE(Buffer_InputSimple) {
    Buffer *buf = new Buffer();

    buf->insert('a');
    BOOST_TEST(buf->getDisplayBuffer() == u8"a");
    BOOST_TEST(buf->getCursor() == 1);

    delete buf;
}

BOOST_AUTO_TEST_CASE(Buffer_TelexSimple) {
    Buffer *buf = new Buffer();
    buf->setToneKeys(ToneKeys::Telex);

    buf->insert('a');
    buf->insert('s');
    BOOST_TEST(buf->getDisplayBuffer() == u8"á");
    BOOST_TEST(buf->getCursor() == 1);

    delete buf;
}

BOOST_AUTO_TEST_CASE(Buffer_Clear) {
    Buffer *buf = new Buffer();

    buf->insert('a');
    buf->clear();
    BOOST_TEST(buf->getDisplayBuffer() == "");
    BOOST_TEST(buf->getCursor() == 0);

    delete buf;
}

BOOST_AUTO_TEST_CASE(Buffer_Telex_Suite) {
    Buffer *buf = new Buffer();
    buf->setToneKeys(ToneKeys::Telex);

    buf->insert('a');
    buf->insert('s');
    BOOST_TEST(buf->getDisplayBuffer() == u8"á");
    BOOST_TEST(buf->getCursor() == 1);
    buf->clear();

    buf->insert('o');
    buf->insert('u');
    BOOST_TEST(buf->getDisplayBuffer() == u8"o͘");
    BOOST_TEST(buf->getCursor() == 2);
    buf->clear();

    buf->insert('o');
    buf->insert('u');
    buf->insert('u');
    BOOST_TEST(buf->getDisplayBuffer() == u8"o u");
    BOOST_TEST(buf->getCursor() == 3);
    buf->clear();

    buf->insert('a');
    buf->insert('s');
    buf->insert('a');
    BOOST_TEST(buf->getDisplayBuffer() == u8"á a");
    BOOST_TEST(buf->getCursor() == 3);
    buf->clear();

    buf->insert('a');
    buf->insert('s');
    buf->insert('b');
    BOOST_TEST(buf->getDisplayBuffer() == u8"á b");
    BOOST_TEST(buf->getCursor() == 3);
    buf->clear();

    buf->insert('a');
    buf->insert('s');
    buf->insert('s');
    BOOST_TEST(buf->getDisplayBuffer() == u8"a s");
    BOOST_TEST(buf->getCursor() == 3);
    buf->clear();

    delete buf;
}

BOOST_AUTO_TEST_CASE(Buffer_MoveCursor) {
    Buffer *buf = new Buffer();
    buf->setToneKeys(ToneKeys::Telex);

    buf->insert('a');
    buf->insert('j');
    buf->insert('a');
    buf->moveCursor(CURS_LEFT);
    buf->insert('s');
    BOOST_TEST(buf->getDisplayBuffer() == u8"ā sa");
    BOOST_TEST(buf->getCursor() == 3);
    buf->clear();

    delete buf;
}

BOOST_AUTO_TEST_SUITE_END();
