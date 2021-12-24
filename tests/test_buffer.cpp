#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "buffer.h"

namespace TaiKey::BufferTest {

struct Fx {
    Fx()
        : db("taikey_with_trie.db"), sp(db.selectSyllableList()),
          tr(db.selectTrieWordlist()), cf(db, sp, tr), buf(cf) {}

    ~Fx() {}
    Buffer buf;
    TKDB db;
    CandidateFinder cf;
    Splitter sp;
    Trie tr;
    void insert(std::string sequence) {
        for (auto it : sequence) {
            buf.insert(it);
        }
    }
};

BOOST_FIXTURE_TEST_SUITE(BufferTest, Fx);

BOOST_AUTO_TEST_CASE(trivial) { BOOST_TEST(true); }

BOOST_AUTO_TEST_CASE(create) {
    BOOST_TEST(buf.getDisplayBuffer() == "");
    BOOST_TEST(buf.getCursor() == 0);
}

BOOST_AUTO_TEST_CASE(clear) {
    buf.setToneKeys(ToneKeys::Telex);
    buf.insert('a');
    buf.clear();
    BOOST_TEST(buf.getDisplayBuffer() == "");
    BOOST_TEST(buf.getCursor() == 0);
}

BOOST_AUTO_TEST_CASE(normal_1_simple) {
    insert("a");
    BOOST_TEST(buf.getDisplayBuffer() == u8"a");
    BOOST_TEST(buf.getCursor() == 1);
}

BOOST_AUTO_TEST_CASE(normal_2_long_string) {
    insert("chetioittengsisekaisiongchanephahjihoat");
    auto disp = buf.getDisplayBuffer();
    BOOST_TEST(disp == "che tio it teng si se kai siong chan e phah jih oat");
    BOOST_TEST(buf.getCursor() == 51);
}

BOOST_AUTO_TEST_CASE(normal_3_with_tones) {
    insert("khiam3eng");
    auto disp = buf.getDisplayBuffer();
    BOOST_TEST(disp == u8"khiàm eng");
    BOOST_TEST(buf.getCursor() == 9);
}

BOOST_AUTO_TEST_CASE(normal_telex_simple) {
    buf.setToneKeys(ToneKeys::Telex);
    insert("as");
    BOOST_TEST(buf.getDisplayBuffer() == u8"á");
    BOOST_TEST(buf.getCursor() == 1);
}

BOOST_AUTO_TEST_CASE(telex_suite) {
    buf.setToneKeys(ToneKeys::Telex);

    insert("as");
    BOOST_TEST(buf.getDisplayBuffer() == u8"á");
    BOOST_TEST(buf.getCursor() == 1);
    buf.clear();

    insert("af");
    BOOST_TEST(buf.getDisplayBuffer() == u8"à");
    BOOST_TEST(buf.getCursor() == 1);
    buf.clear();

    insert("al");
    BOOST_TEST(buf.getDisplayBuffer() == u8"â");
    BOOST_TEST(buf.getCursor() == 1);
    buf.clear();

    insert("aj");
    BOOST_TEST(buf.getDisplayBuffer() == u8"ā");
    BOOST_TEST(buf.getCursor() == 1);
    buf.clear();

    insert("ahj");
    BOOST_TEST(buf.getDisplayBuffer() == u8"a̍h");
    BOOST_TEST(buf.getCursor() == 3);
    buf.clear();

    insert("ou");
    BOOST_TEST(buf.getDisplayBuffer() == u8"o͘");
    BOOST_TEST(buf.getCursor() == 2);
    buf.clear();

    insert("ouu");
    BOOST_TEST(buf.getDisplayBuffer() == u8"o u");
    BOOST_TEST(buf.getCursor() == 3);
    buf.clear();

    insert("asa");
    BOOST_TEST(buf.getDisplayBuffer() == u8"á a");
    BOOST_TEST(buf.getCursor() == 3);
    buf.clear();

    insert("asb");
    BOOST_TEST(buf.getDisplayBuffer() == u8"á b");
    BOOST_TEST(buf.getCursor() == 3);
    buf.clear();

    insert("ass");
    BOOST_TEST(buf.getDisplayBuffer() == u8"a s");
    BOOST_TEST(buf.getCursor() == 3);
    buf.clear();
}

BOOST_AUTO_TEST_CASE(move_cursor) {
    buf.setToneKeys(ToneKeys::Telex);
    insert("a");
    buf.moveCursor(CursorDirection::L);
    BOOST_TEST(buf.getCursor() == 0);
    buf.moveCursor(CursorDirection::L);
    BOOST_TEST(buf.getCursor() == 0);
    buf.moveCursor(CursorDirection::R);
    BOOST_TEST(buf.getCursor() == 1);
    buf.moveCursor(CursorDirection::R);
    BOOST_TEST(buf.getCursor() == 1);
    buf.clear();

    insert("aja");
    buf.moveCursor(CursorDirection::L);
    BOOST_TEST(buf.getCursor() == 2);
}

BOOST_AUTO_TEST_CASE(move_cursor_and_insert) {
    buf.setToneKeys(ToneKeys::Telex);
    insert("aja");
    buf.moveCursor(CursorDirection::L);
    buf.insert('s');
    BOOST_TEST(buf.getDisplayBuffer() == u8"ā sa");
    BOOST_TEST(buf.getCursor() == 3);
    buf.clear();
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace TaiKey::BufferTest