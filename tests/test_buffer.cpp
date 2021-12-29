#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "buffer.h"

namespace TaiKey::BufferTest {

struct Fx {
    Fx()
        : db("taikey.db"), sp(db.selectSyllableList()),
          tr(db.selectTrieWordlist(), db.selectSyllableList()), cf(db, sp, tr),
          buf(cf) {}

    ~Fx() {}
    BufferManager buf;
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

BOOST_AUTO_TEST_CASE(normal_4_with_hyphens_gisu) {
    insert("khiam3-eng7");
    auto disp = buf.getDisplayBuffer();
    BOOST_TEST(disp == u8"khiàm-ēng");
    BOOST_TEST(buf.getCursor() == 9);
}

BOOST_AUTO_TEST_CASE(normal_5_with_hyphens_no_gisu) {
    insert("chiah8-png7");
    auto disp = buf.getDisplayBuffer();
    BOOST_TEST(disp == u8"chia̍h-pn̄g");
    BOOST_TEST(buf.getCursor() == 11);
}

BOOST_AUTO_TEST_CASE(normal_6_move_cursor_and_type) {
    insert("siongho");

    auto disp = buf.getDisplayBuffer();
    BOOST_TEST(disp == "siong ho");
    BOOST_TEST(buf.getCursor() == 8);

    buf.moveCursor(CursorDirection::L);
    buf.moveCursor(CursorDirection::L);
    buf.moveCursor(CursorDirection::L);
    buf.moveCursor(CursorDirection::L);
    buf.moveCursor(CursorDirection::L);
    buf.moveCursor(CursorDirection::L);
    buf.insert('h');

    disp = buf.getDisplayBuffer();
    BOOST_TEST(disp == "sih ong ho");
    BOOST_TEST(buf.getCursor() == 4);
}

BOOST_AUTO_TEST_CASE(normal_7_remove_chars) {
    insert("siongho");
    buf.remove(CursorDirection::L);

    auto disp = buf.getDisplayBuffer();
    BOOST_TEST(disp == "siong h");
    BOOST_TEST(buf.getCursor() == 7);

    buf.moveCursor(CursorDirection::L);
    buf.remove(CursorDirection::R);

    disp = buf.getDisplayBuffer();
    BOOST_TEST(disp == "siong");
    BOOST_TEST(buf.getCursor() == 5);
}

BOOST_AUTO_TEST_CASE(normal_8_random_letters) {
    insert("bo5wdprsfnlji7");

    auto disp = buf.getDisplayBuffer();
    BOOST_TEST(disp == "bô wdprsfnl jī");
    BOOST_TEST(buf.getCursor() == 14);
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
    BOOST_TEST(buf.getCursor() == 3);
    buf.moveCursor(CursorDirection::L);
    BOOST_TEST(buf.getCursor() == 2);
    buf.clear();

    insert("ah8");
    BOOST_TEST(buf.getCursor() == 3);
    buf.moveCursor(CursorDirection::L);
    BOOST_TEST(buf.getCursor() == 2);
    buf.moveCursor(CursorDirection::L);
    BOOST_TEST(buf.getCursor() == 0);
}

BOOST_AUTO_TEST_CASE(move_cursor_and_insert) {
    insert("aja");
    buf.moveCursor(CursorDirection::L);
    insert("s");
    BOOST_TEST(buf.getDisplayBuffer() == u8"a j sa");
    BOOST_TEST(buf.getCursor() == 5);
    buf.clear();
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace TaiKey::BufferTest
