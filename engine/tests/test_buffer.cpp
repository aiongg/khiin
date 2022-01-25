#include <gtest/gtest.h>

#include "BufferManager.h"

namespace khiin::engine {
namespace {

const std::string DB_FILE = "taikey.db";

class BufferFx : public ::testing::Test {
  protected:
    void SetUp() override {
        db = new Database(DB_FILE);
        auto sylList = db->GetSyllableList();
        sp = new Splitter(sylList), tr = new Trie(db->GetTrieWordlist(), sylList);
        cf = new CandidateFinder(db, sp, tr);
        buf = BufferManager::Create(cf);
    }

    ~BufferFx() {
        delete db;
        delete sp;
        delete tr;
        delete cf;
        delete buf;
    }

    BufferManager *buf = nullptr;
    Database *db = nullptr;
    CandidateFinder *cf = nullptr;
    Splitter *sp = nullptr;
    Trie *tr = nullptr;

    void insert(std::string sequence) {
        for (auto it : sequence) {
            buf->Insert(it);
        }
    }
    void left(int n) {
        for (auto i = 0; i < n; i++) {
            buf->MoveCaret(CursorDirection::L);
        }
    }
    void right(int n) {
        for (auto i = 0; i < n; i++) {
            buf->MoveCaret(CursorDirection::R);
        }
    }
    void bksp(int n) {
        for (auto i = 0; i < n; i++) {
            buf->Erase(CursorDirection::L);
        }
    }
    void del(int n) {
        for (auto i = 0; i < n; i++) {
            buf->Erase(CursorDirection::R);
        }
    }
    void reset() {
        buf->Clear();
    }
    std::string getBuf() {
        return buf->getDisplayBuffer();
    }
    std::size_t getCurs() {
        return buf->getCursor();
    }
};

TEST_F(BufferFx, Trivial) {
    EXPECT_TRUE(true);
}

TEST_F(BufferFx, Create) {
    EXPECT_EQ(getBuf(), "");
    EXPECT_EQ(getCurs(), 0);
}

TEST_F(BufferFx, Reset) {
    insert("a");
    reset();
    EXPECT_EQ(getBuf(), "");
    EXPECT_EQ(getCurs(), 0);
}

TEST_F(BufferFx, Simple) {
    insert("a");
    EXPECT_EQ(getBuf(), u8"a");
    EXPECT_EQ(getCurs(), 1);
}

TEST_F(BufferFx, LongString) {
    insert("chetioittengsisekaisiongchanephahjihoat");
    EXPECT_EQ(getBuf(), "che tio it teng si se kai siong chan e phah jih oat");
    EXPECT_EQ(getCurs(), 51);
}

TEST_F(BufferFx, WithTones) {
    insert("khiam3eng");
    EXPECT_EQ(getBuf(), u8"khiàm eng");
    EXPECT_EQ(getCurs(), 9);
}

TEST_F(BufferFx, WithHyphenGisu) {
    insert("khiam3-eng7");
    EXPECT_EQ(getBuf(), u8"khiàm-ēng");
    EXPECT_EQ(getCurs(), 9);
}

TEST_F(BufferFx, CustomHyphenGisu) {
    insert("chiah8-png7");
    EXPECT_EQ(getBuf(), u8"chia̍h-pn̄g");
    EXPECT_EQ(getCurs(), 11);
}

TEST_F(BufferFx, MoveCursorAndType) {
    insert("siongho");

    EXPECT_EQ(getBuf(), "siong ho");
    EXPECT_EQ(getCurs(), 8);

    left(6);
    insert("h");

    EXPECT_EQ(getBuf(), "sih ong ho");
    EXPECT_EQ(getCurs(), 3);
}

TEST_F(BufferFx, t07_remove_chars) {
    insert("siongho");

    bksp(1);
    EXPECT_EQ(getBuf(), "siong h");
    EXPECT_EQ(getCurs(), 7);

    bksp(1);
    EXPECT_EQ(getBuf(), "siong");
    EXPECT_EQ(getCurs(), 5);

    bksp(1);
    EXPECT_EQ(getBuf(), "sion");
    EXPECT_EQ(getCurs(), 4);

    bksp(1);
    EXPECT_EQ(getBuf(), "sio");
    EXPECT_EQ(getCurs(), 3);

    reset();
    insert("siongho");

    left(2);

    del(1);
    EXPECT_EQ(getBuf(), "siong o");
    EXPECT_EQ(getCurs(), 6);

    del(1);
    EXPECT_EQ(getBuf(), "siong");
    EXPECT_EQ(getCurs(), 5);

    reset();
    insert("siongho");
    left(2);

    EXPECT_EQ(getBuf(), "siong ho");
    EXPECT_EQ(getCurs(), 6);

    bksp(1);

    EXPECT_EQ(getBuf(), "siong ho");
    EXPECT_EQ(getCurs(), 5);

    reset();
    insert("khi3");
    EXPECT_EQ(getBuf(), u8"khì");
    EXPECT_EQ(getCurs(), 3);

    bksp(1);

    EXPECT_EQ(getBuf(), u8"kh");
    EXPECT_EQ(getCurs(), 2);
}

TEST_F(BufferFx, t08_random_letters) {
    insert("bo5wdprsfnlji7");

    EXPECT_EQ(getBuf(), "bô wdprsfnl jī");
    EXPECT_EQ(getCurs(), 14);

    left(2);
    bksp(9);

    EXPECT_EQ(getBuf(), "bô jī");
    EXPECT_EQ(getCurs(), 2);
}

TEST_F(BufferFx, t09_remove_tones) {
    insert("kah8a");
    EXPECT_EQ(getBuf(), u8"ka̍h a");
    EXPECT_EQ(getCurs(), 6);
    left(3);
    EXPECT_EQ(getCurs(), 3);
    bksp(1);
    EXPECT_EQ(getBuf(), "kha");
    EXPECT_EQ(getCurs(), 1);
}

TEST_F(BufferFx, t10_ur_or) {
    insert("ur");
    EXPECT_EQ(getBuf(), u8"\u1e73");
    EXPECT_EQ(getCurs(), 1);

    reset();

    insert("or");
    EXPECT_EQ(getBuf(), u8"o\u0324");
    EXPECT_EQ(getCurs(), 2);
}

TEST_F(BufferFx, t11_khin) {
    insert("ho--a");
    EXPECT_EQ(getBuf(), "ho ·a");
    EXPECT_EQ(getCurs(), 5);

    left(1);
    bksp(1);
    EXPECT_EQ(getBuf(), "ho a");
    EXPECT_EQ(getCurs(), 3);

    reset();
    insert("siak--lohkhi");
    EXPECT_EQ(getBuf(), "siak ·loh ·khi");

    reset();

    insert("--a");
    EXPECT_EQ(getBuf(), "·a");

    reset();
    insert("pinn---a");
    EXPECT_EQ(getBuf(), "piⁿ-·a");
}

TEST_F(BufferFx, t12_select_primary_candidate) {
    insert("ho2");
    buf->selectPrimaryCandidate();
    EXPECT_EQ(getBuf(), u8"好");
    EXPECT_EQ(getCurs(), 1);
}

TEST_F(BufferFx, t13_long_candidate) {
    insert("kutkutkutkutkutkut");
    auto cand = buf->getCandidates();
    EXPECT_EQ(cand[0].text, u8"骨骨骨骨骨骨");
}

// BOOST_AUTO_TEST_CASE(ttelex_simple) {
//    buf->setToneKeys(ToneKeys::Telex);
//    insert("as");
//    EXPECT_EQ(getBuf() ,  u8"á");
//    EXPECT_EQ(getCurs() ,  1);
//}

// BOOST_AUTO_TEST_CASE(telex_suite) {
//    buf->setToneKeys(ToneKeys::Telex);
//
//    insert("as");
//    EXPECT_EQ(getBuf() ,  u8"á");
//    EXPECT_EQ(getCurs() ,  1);
//    reset();
//
//    insert("af");
//    EXPECT_EQ(getBuf() ,  u8"à");
//    EXPECT_EQ(getCurs() ,  1);
//    reset();
//
//    insert("al");
//    EXPECT_EQ(getBuf() ,  u8"â");
//    EXPECT_EQ(getCurs() ,  1);
//    reset();
//
//    insert("aj");
//    EXPECT_EQ(getBuf() ,  u8"ā");
//    EXPECT_EQ(getCurs() ,  1);
//    reset();
//
//    insert("ahj");
//    EXPECT_EQ(getBuf() ,  u8"a̍h");
//    EXPECT_EQ(getCurs() ,  3);
//    reset();
//
//    insert("ou");
//    EXPECT_EQ(getBuf() ,  u8"o͘");
//    EXPECT_EQ(getCurs() ,  2);
//    reset();
//
//    insert("ouu");
//    EXPECT_EQ(getBuf() ,  u8"o u");
//    EXPECT_EQ(getCurs() ,  3);
//    reset();
//
//    insert("asa");
//    EXPECT_EQ(getBuf() ,  u8"á a");
//    EXPECT_EQ(getCurs() ,  3);
//    reset();
//
//    insert("asb");
//    EXPECT_EQ(getBuf() ,  u8"á b");
//    EXPECT_EQ(getCurs() ,  3);
//    reset();
//
//    insert("ass");
//    EXPECT_EQ(getBuf() ,  u8"a s");
//    EXPECT_EQ(getCurs() ,  3);
//    reset();
//}

TEST_F(BufferFx, move_cursor) {
    insert("a");
    left(1);
    EXPECT_EQ(getCurs(), 0);
    left(1);
    EXPECT_EQ(getCurs(), 0);
    right(1);
    EXPECT_EQ(getCurs(), 1);
    right(1);
    EXPECT_EQ(getCurs(), 1);
    reset();

    insert("aja");
    left(1);
    EXPECT_EQ(getCurs(), 3);
    left(1);
    EXPECT_EQ(getCurs(), 2);
    reset();

    insert("ah8");
    EXPECT_EQ(getCurs(), 3);
    left(1);
    EXPECT_EQ(getCurs(), 2);
    left(1);
    EXPECT_EQ(getCurs(), 0);

    reset();
    insert("ouhnn8");
    EXPECT_EQ(getCurs(), 5);
    left(1);
    EXPECT_EQ(getCurs(), 4);
    left(1);
    EXPECT_EQ(getCurs(), 3);
    left(1);
    EXPECT_EQ(getCurs(), 0);
    right(1);
    EXPECT_EQ(getCurs(), 3);
    right(1);
    EXPECT_EQ(getCurs(), 4);
    right(1);
    EXPECT_EQ(getCurs(), 5);
}

TEST_F(BufferFx, move_cursor_and_insert) {
    insert("aja");
    left(1);
    insert("s");
    EXPECT_EQ(getBuf(), u8"a j sa");
    EXPECT_EQ(getCurs(), 5);
    reset();
}

// TEST_F(BufferFx, multi_test) {
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

} // namespace
} // namespace khiin::engine
