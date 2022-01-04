#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "buffer_manager.h"

namespace TaiKey::BufferTest {

const std::string DB_FILE = "taikey.db";

struct BufferFx {
    BufferFx()
        : db(DB_FILE), sp(db.selectSyllableList()),
          tr(db.selectTrieWordlist(), db.selectSyllableList()), cf(db, sp, tr),
          buf(cf) {}

    ~BufferFx() {}
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
    void left(int n) {
        for (auto i = 0; i < n; i++) {
            buf.moveCursor(CursorDirection::L);
        }
    }
    void right(int n) {
        for (auto i = 0; i < n; i++) {
            buf.moveCursor(CursorDirection::R);
        }
    }
    void bksp(int n) {
        for (auto i = 0; i < n; i++) {
            buf.erase(CursorDirection::L);
        }
    }
    void del(int n) {
        for (auto i = 0; i < n; i++) {
            buf.erase(CursorDirection::R);
        }
    }
    void reset() { buf.clear(); }
    std::string getBuf() { return buf.getDisplayBuffer(); }
    std::size_t getCurs() { return buf.getCursor(); }
};

BOOST_FIXTURE_TEST_SUITE(BufferTest, BufferFx);

BOOST_AUTO_TEST_CASE(trivial) { BOOST_TEST(true); }

BOOST_AUTO_TEST_CASE(create) {
    BOOST_TEST(getBuf() == "");
    BOOST_TEST(getCurs() == 0);
}

BOOST_AUTO_TEST_CASE(clear) {
    buf.setToneKeys(ToneKeys::Telex);
    insert("a");
    reset();
    BOOST_TEST(getBuf() == "");
    BOOST_TEST(getCurs() == 0);
}

BOOST_AUTO_TEST_CASE(spacebar_not_consumed) {
    BOOST_TEST((buf.spacebar() == RetVal::NotConsumed));
}

BOOST_AUTO_TEST_CASE(t01_simple) {
    insert("a");
    BOOST_TEST(getBuf() == u8"a");
    BOOST_TEST(getCurs() == 1);
}

BOOST_AUTO_TEST_CASE(t02_long_string) {
    insert("chetioittengsisekaisiongchanephahjihoat");
    BOOST_TEST(getBuf() ==
               "che tio it teng si se kai siong chan e phah jih oat");
    BOOST_TEST(getCurs() == 51);
}

BOOST_AUTO_TEST_CASE(t03_with_tones) {
    insert("khiam3eng");
    BOOST_TEST(getBuf() == u8"khiàm eng");
    BOOST_TEST(getCurs() == 9);
}

BOOST_AUTO_TEST_CASE(t04_with_hyphens_gisu) {
    insert("khiam3-eng7");
    BOOST_TEST(getBuf() == u8"khiàm-ēng");
    BOOST_TEST(getCurs() == 9);
}

BOOST_AUTO_TEST_CASE(t05_with_hyphens_no_gisu) {
    insert("chiah8-png7");
    BOOST_TEST(getBuf() == u8"chia̍h-pn̄g");
    BOOST_TEST(getCurs() == 11);
}

BOOST_AUTO_TEST_CASE(t06_move_cursor_and_type) {
    insert("siongho");

    BOOST_TEST(getBuf() == "siong ho");
    BOOST_TEST(getCurs() == 8);

    left(6);
    insert("h");

    BOOST_TEST(getBuf() == "sih ong ho");
    BOOST_TEST(getCurs() == 3);
}

BOOST_AUTO_TEST_CASE(t07_remove_chars) {
    insert("siongho");

    bksp(1);
    BOOST_TEST(getBuf() == "siong h");
    BOOST_TEST(getCurs() == 7);

    bksp(1);
    BOOST_TEST(getBuf() == "siong");
    BOOST_TEST(getCurs() == 5);

    bksp(1);
    BOOST_TEST(getBuf() == "sion");
    BOOST_TEST(getCurs() == 4);

    bksp(1);
    BOOST_TEST(getBuf() == "sio");
    BOOST_TEST(getCurs() == 3);

    buf.clear();
    insert("siongho");

    left(2);

    del(1);
    BOOST_TEST(getBuf() == "siong o");
    BOOST_TEST(getCurs() == 6);

    del(1);
    BOOST_TEST(getBuf() == "siong");
    BOOST_TEST(getCurs() == 5);

    buf.clear();
    insert("siongho");
    left(2);

    BOOST_TEST(getBuf() == "siong ho");
    BOOST_TEST(getCurs() == 6);

    bksp(1);

    BOOST_TEST(getBuf() == "siong ho");
    BOOST_TEST(getCurs() == 5);

    buf.clear();

    insert("khi3");
    BOOST_TEST(getBuf() == u8"khì");
    BOOST_TEST(getCurs() == 3);

    bksp(1);

    BOOST_TEST(getBuf() == u8"kh");
    BOOST_TEST(getCurs() == 2);
}

BOOST_AUTO_TEST_CASE(t08_random_letters) {
    insert("bo5wdprsfnlji7");

    BOOST_TEST(getBuf() == "bô wdprsfnl jī");
    BOOST_TEST(getCurs() == 14);

    left(2);
    bksp(9);

    BOOST_TEST(getBuf() == "bô jī");
    BOOST_TEST(getCurs() == 2);
}

BOOST_AUTO_TEST_CASE(t09_remove_tones) {
    insert("kah8a");
    BOOST_TEST(getBuf() == u8"ka̍h a");
    BOOST_TEST(getCurs() == 6);
    left(3);
    BOOST_TEST(getCurs() == 3);
    bksp(1);
    BOOST_TEST(getBuf() == "kha");
    BOOST_TEST(getCurs() == 1);
}

BOOST_AUTO_TEST_CASE(t10_ur_or) {
    insert("ur");
    BOOST_TEST(getBuf() == u8"\u1e73");
    BOOST_TEST(getCurs() == 1);

    reset();

    insert("or");
    BOOST_TEST(getBuf() == u8"o\u0324");
    BOOST_TEST(getCurs() == 2);
}

BOOST_AUTO_TEST_CASE(t11_khin) {
    insert("ho--a");
    BOOST_TEST(getBuf() == "ho ·a");
    BOOST_TEST(getCurs() == 5);

    left(1);
    bksp(1);
    BOOST_TEST(getBuf() == "ho a");
    BOOST_TEST(getCurs() == 3);

    reset();
    insert("siak--lohkhi");
    BOOST_TEST(getBuf() == "siak ·loh ·khi");

    reset();

    insert("--a");
    BOOST_TEST(getBuf() == "·a");

    reset();
    insert("pinn---a");
    BOOST_TEST(getBuf() == "piⁿ-·a");
}

BOOST_AUTO_TEST_CASE(t12_select_primary_candidate) {
    insert("ho2");
    buf.spacebar();
    BOOST_TEST(getBuf() == u8"好");
    BOOST_TEST(getCurs() == 1);
}

BOOST_AUTO_TEST_CASE(ttelex_simple) {
    buf.setToneKeys(ToneKeys::Telex);
    insert("as");
    BOOST_TEST(getBuf() == u8"á");
    BOOST_TEST(getCurs() == 1);
}

BOOST_AUTO_TEST_CASE(telex_suite) {
    buf.setToneKeys(ToneKeys::Telex);

    insert("as");
    BOOST_TEST(getBuf() == u8"á");
    BOOST_TEST(getCurs() == 1);
    reset();

    insert("af");
    BOOST_TEST(getBuf() == u8"à");
    BOOST_TEST(getCurs() == 1);
    reset();

    insert("al");
    BOOST_TEST(getBuf() == u8"â");
    BOOST_TEST(getCurs() == 1);
    reset();

    insert("aj");
    BOOST_TEST(getBuf() == u8"ā");
    BOOST_TEST(getCurs() == 1);
    reset();

    insert("ahj");
    BOOST_TEST(getBuf() == u8"a̍h");
    BOOST_TEST(getCurs() == 3);
    reset();

    insert("ou");
    BOOST_TEST(getBuf() == u8"o͘");
    BOOST_TEST(getCurs() == 2);
    reset();

    insert("ouu");
    BOOST_TEST(getBuf() == u8"o u");
    BOOST_TEST(getCurs() == 3);
    reset();

    insert("asa");
    BOOST_TEST(getBuf() == u8"á a");
    BOOST_TEST(getCurs() == 3);
    reset();

    insert("asb");
    BOOST_TEST(getBuf() == u8"á b");
    BOOST_TEST(getCurs() == 3);
    reset();

    insert("ass");
    BOOST_TEST(getBuf() == u8"a s");
    BOOST_TEST(getCurs() == 3);
    reset();
}

BOOST_AUTO_TEST_CASE(move_cursor) {
    insert("a");
    left(1);
    BOOST_TEST(getCurs() == 0);
    left(1);
    BOOST_TEST(getCurs() == 0);
    right(1);
    BOOST_TEST(getCurs() == 1);
    right(1);
    BOOST_TEST(getCurs() == 1);
    reset();

    insert("aja");
    left(1);
    BOOST_TEST(getCurs() == 3);
    left(1);
    BOOST_TEST(getCurs() == 2);
    reset();

    insert("ah8");
    BOOST_TEST(getCurs() == 3);
    left(1);
    BOOST_TEST(getCurs() == 2);
    left(1);
    BOOST_TEST(getCurs() == 0);

    reset();
    insert("ouhnn8");
    BOOST_TEST(getCurs() == 5);
    left(1);
    BOOST_TEST(getCurs() == 4);
    left(1);
    BOOST_TEST(getCurs() == 3);
    left(1);
    BOOST_TEST(getCurs() == 0);
    right(1);
    BOOST_TEST(getCurs() == 3);
    right(1);
    BOOST_TEST(getCurs() == 4);
    right(1);
    BOOST_TEST(getCurs() == 5);
}

BOOST_AUTO_TEST_CASE(move_cursor_and_insert) {
    insert("aja");
    left(1);
    insert("s");
    BOOST_TEST(getBuf() == u8"a j sa");
    BOOST_TEST(getCurs() == 5);
    reset();
}

// BOOST_AUTO_TEST_CASE(multi_test) {
//    auto test_strings = std::vector<std::string>{"itannalainia",
//                                                 "iaumchiaheanne",
//                                                 "iauchiaheanne",
//                                                 "iiauchiaheanneou",
//                                                 "iniauchiaheanneou",
//                                                 "naapinnhiaboa",
//                                                 "naapinnhiaaboeu",
//                                                 "naapinnhiaabeu",
//                                                 "khachiahauoe",
//                                                 "simsianne",
//                                                 "simsekiaho",
//                                                 "tiasitihia",
//                                                 "inaesitihia",
//                                                 "inahesitihia",
//                                                 "oathng",
//                                                 "chiahoathng",
//                                                 "oatihaite",
//                                                 "ouatihaite",
//                                                 "chiteaichiahoa",
//                                                 "chiteaichiahoua",
//                                                 "oanaesitihia",
//                                                 "oanahesitihia",
//                                                 "ahanahesitihia",
//                                                 "chitchiahaha",
//                                                 "chitboehia",
//                                                 "chitbehia",
//                                                 "hianaheboboe",
//                                                 "hianahebobe",
//                                                 "kannaaitihoa",
//                                                 "sinithiann",
//                                                 "sisinithiann",
//                                                 "chhiti",
//                                                 "chhitia",
//                                                 "poetemthangchhikimhi",
//                                                 "ohhouho",
//                                                 "ohhooho",
//                                                 "aiohhouhoou",
//                                                 "aiohhoohooo",
//                                                 "aiohhoohoou",
//                                                 "aiohhouhooo",
//                                                 "ohhouhoaboe",
//                                                 "ohhoohoaboe",
//                                                 "ohhoohoabe"};
//
//    for (auto &s : test_strings) {
//        reset();
//        insert(s);
//    }
//}

BOOST_AUTO_TEST_SUITE_END();

} // namespace TaiKey::BufferTest
