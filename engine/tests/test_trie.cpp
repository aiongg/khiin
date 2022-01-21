#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <memory>

#include "trie.h"

using namespace std;
using namespace khiin::engine;

struct TrieFx {
    TrieFx() { trie = new Trie(); }
    ~TrieFx() { delete trie; }
    Trie *trie = nullptr;
    void ins(std::vector<std::string> words) {
        for (auto &it : words) {
            trie->insert(it);
        }
    }
};

BOOST_FIXTURE_TEST_SUITE(TrieTest, TrieFx);

BOOST_AUTO_TEST_CASE(search) {
    trie->insert(u8"niau");
    bool res = trie->containsWord(u8"niau");

    BOOST_TEST(res == true);
}

BOOST_AUTO_TEST_CASE(remove) {
    trie->insert(u8"niau");
    trie->insert(u8"ni");
    trie->remove(u8"niau");

    BOOST_TEST(!trie->containsWord(u8"niau"));
    BOOST_TEST(!trie->containsWord(u8"nia"));
    BOOST_TEST(trie->containsWord(u8"ni"));
    BOOST_TEST(!trie->containsWord(u8"n"));
}

BOOST_AUTO_TEST_CASE(is_prefix) {
    trie->insert(u8"niau");
    BOOST_TEST(trie->containsPrefix(u8"nia") == true);
}

BOOST_AUTO_TEST_CASE(autocomplete) {
    ins({u8"niau", u8"nia", u8"na"});

    std::vector<std::string> res = trie->autocomplete(u8"n");
    BOOST_TEST(res.size() == 3);
    BOOST_TEST((std::find(res.begin(), res.end(), "niau") != res.end()));
    BOOST_TEST((std::find(res.begin(), res.end(), "nia") != res.end()));
    BOOST_TEST((std::find(res.begin(), res.end(), "na") != res.end()));

    res = trie->autocomplete(u8"na");
    BOOST_TEST(res.size() == 1);
    BOOST_TEST((std::find(res.begin(), res.end(), "na") != res.end()));
}

BOOST_AUTO_TEST_CASE(autocomplete_tone) {
    ins({u8"na2", u8"na7", u8"nai"});

    std::vector<std::string> res = trie->autocompleteTone(u8"na");
    BOOST_TEST(res.size() == 2);
    BOOST_TEST((std::find(res.begin(), res.end(), "na2") != res.end()));
    BOOST_TEST((std::find(res.begin(), res.end(), "na7") != res.end()));
}

BOOST_AUTO_TEST_CASE(get_all_words) {
    ins({"cho", "cho2", "chong", "chong5", "chongthong2", "ba"});
    auto res = VStr();
    trie->getAllWords("chongthong", true, res);
    BOOST_TEST(res.size() == 5);
    BOOST_TEST((std::find(res.begin(), res.end(), "cho2") != res.end()));
}

/**
* input: limaianne
1. Lí mài án-ne.
2. Lí mā iâⁿ--ê.
3. Lí mā ian--ne.
4. Lîm-á iâⁿ--ê.
5. Lim ài an--ne.
6. Lim ài án-ne.
*/
BOOST_AUTO_TEST_CASE(sentence_split) {
    ins({"li", "mai", "an", "ne", "ma", "ian", "iann", "lim", "ai", "e",
         "anne"});
    auto res = trie->splitSentence2("limaianne");
    int i = 0;
}

BOOST_AUTO_TEST_CASE(big_word_list) {
    std::vector<std::string> w;
    w.push_back("a");
    w.push_back("ah");
    w.push_back("ahnn");
    w.push_back("ai");
    w.push_back("aih");
    w.push_back("ainn");
    w.push_back("ak");
    w.push_back("am");
    w.push_back("an");
    w.push_back("ang");
    w.push_back("ann");
    w.push_back("ap");
    w.push_back("ar");
    w.push_back("at");
    w.push_back("au");
    w.push_back("aunn");
    w.push_back("ba");
    w.push_back("bah");
    w.push_back("bai");
    w.push_back("bak");
    w.push_back("bam");
    w.push_back("ban");
    w.push_back("bang");
    w.push_back("bat");
    w.push_back("bau");
    w.push_back("bauh");
    w.push_back("be");
    w.push_back("beh");
    w.push_back("bek");
    w.push_back("beng");
    w.push_back("bi");
    w.push_back("bian");
    w.push_back("biat");
    w.push_back("biau");
    w.push_back("bih");
    w.push_back("bim");
    w.push_back("bin");
    w.push_back("bio");
    w.push_back("bit");
    w.push_back("biu");
    w.push_back("bo");
    w.push_back("boa");
    w.push_back("boah");
    w.push_back("boai");
    w.push_back("boaih");
    w.push_back("boan");
    w.push_back("boat");
    w.push_back("boe");
    w.push_back("boeh");
    w.push_back("bok");
    w.push_back("bong");
    w.push_back("bor");
    w.push_back("borh");
    w.push_back("bou");
    w.push_back("bu");
    w.push_back("buh");
    w.push_back("bui");
    w.push_back("buk");
    w.push_back("bun");
    w.push_back("but");
    w.push_back("cha");
    w.push_back("chah");
    w.push_back("chai");
    w.push_back("chaih");
    w.push_back("chaik");
    w.push_back("chaim");
    w.push_back("chainn");
    w.push_back("chak");
    w.push_back("cham");
    w.push_back("chan");
    w.push_back("chang");
    w.push_back("chann");
    w.push_back("chap");
    w.push_back("chat");
    w.push_back("chau");
    w.push_back("che");
    w.push_back("cheh");
    w.push_back("chek");
    w.push_back("cheng");
    w.push_back("chenn");
    w.push_back("chha");
    w.push_back("chhah");
    w.push_back("chhai");
    w.push_back("chhainn");
    w.push_back("chhak");
    w.push_back("chham");
    w.push_back("chhan");
    w.push_back("chhang");
    w.push_back("chhann");
    w.push_back("chhap");
    w.push_back("chhat");
    w.push_back("chhau");
    w.push_back("chhauh");
    w.push_back("chhe");
    w.push_back("chheh");
    w.push_back("chhek");
    w.push_back("chheng");
    w.push_back("chhenn");
    w.push_back("chhi");
    w.push_back("chhia");
    w.push_back("chhiah");
    w.push_back("chhiak");
    w.push_back("chhiam");
    w.push_back("chhian");
    w.push_back("chhiang");
    w.push_back("chhiann");
    w.push_back("chhiat");
    w.push_back("chhiau");
    w.push_back("chhih");
    w.push_back("chhihnn");
    w.push_back("chhim");
    w.push_back("chhin");
    w.push_back("chhina");
    w.push_back("chhinn");
    w.push_back("chhio");
    w.push_back("chhioh");
    w.push_back("chhiok");
    w.push_back("chhiong");
    w.push_back("chhiou");
    w.push_back("chhip");
    w.push_back("chhit");
    w.push_back("chhiu");
    w.push_back("chhiue");
    w.push_back("chhiunn");
    w.push_back("chhneng");
    w.push_back("chhng");
    w.push_back("chhngh");
    w.push_back("chho");
    w.push_back("chhoa");
    w.push_back("chhoah");
    w.push_back("chhoai");
    w.push_back("chhoaih");
    w.push_back("chhoan");
    w.push_back("chhoann");
    w.push_back("chhoe");
    w.push_back("chhoeh");
    w.push_back("chhoh");
    w.push_back("chhok");
    w.push_back("chhong");
    w.push_back("chhonn");
    w.push_back("chhop");
    w.push_back("chhor");
    w.push_back("chhorh");
    w.push_back("chhou");
    w.push_back("chhu");
    w.push_back("chhuh");
    w.push_back("chhui");
    w.push_back("chhuinn");
    w.push_back("chhun");
    w.push_back("chhur");
    w.push_back("chhut");
    w.push_back("chi");
    w.push_back("chia");
    w.push_back("chiaan");
    w.push_back("chiah");
    w.push_back("chiah…");
    w.push_back("chiahnn");
    w.push_back("chiai");
    w.push_back("chiam");
    w.push_back("chian");
    w.push_back("chiang");
    w.push_back("chiann");
    w.push_back("chiap");
    w.push_back("chiat");
    w.push_back("chiau");
    w.push_back("chiauh");
    w.push_back("chih");
    w.push_back("chim");
    w.push_back("chin");
    w.push_back("chinn");
    w.push_back("chio");
    w.push_back("chioh");
    w.push_back("chiok");
    w.push_back("chiong");
    w.push_back("chiou");
    w.push_back("chip");
    w.push_back("chit");
    w.push_back("chiu");
    w.push_back("chiuan");
    w.push_back("chiuh");
    w.push_back("chiunn");
    w.push_back("chiut");
    w.push_back("chng");
    w.push_back("cho");
    w.push_back("choa");
    w.push_back("choah");
    w.push_back("choai");
    w.push_back("choainn");
    w.push_back("choan");
    w.push_back("choann");
    w.push_back("choat");
    w.push_back("choe");
    w.push_back("choeh");
    w.push_back("choh");
    w.push_back("chok");
    w.push_back("chong");
    w.push_back("chop");
    w.push_back("chor");
    w.push_back("chou");
    w.push_back("chu");
    w.push_back("chua");
    w.push_back("chuh");
    w.push_back("chui");
    w.push_back("chuinn");
    w.push_back("chun");
    w.push_back("chur");
    w.push_back("churh");
    w.push_back("chut");
    w.push_back("chuui");
    w.push_back("da");
    w.push_back("dat");
    w.push_back("de");
    w.push_back("dem");
    w.push_back("di");
    w.push_back("do");
    w.push_back("dou");
    w.push_back("e");
    w.push_back("eh");
    w.push_back("ehnn");
    w.push_back("ek");
    w.push_back("em");
    w.push_back("eng");
    w.push_back("enn");
    w.push_back("ep");
    w.push_back("fa");
    w.push_back("fak");
    w.push_back("fe");
    w.push_back("fo");
    w.push_back("for");
    w.push_back("fou");
    w.push_back("fu");
    w.push_back("fuh");
    w.push_back("furn");
    w.push_back("ga");
    w.push_back("gah");
    w.push_back("gai");
    w.push_back("gak");
    w.push_back("gam");
    w.push_back("gan");
    w.push_back("gang");
    w.push_back("gap");
    w.push_back("gau");
    w.push_back("ge");
    w.push_back("geh");
    w.push_back("gek");
    w.push_back("geng");
    w.push_back("gi");
    w.push_back("gia");
    w.push_back("giah");
    w.push_back("giak");
    w.push_back("giam");
    w.push_back("gian");
    w.push_back("giang");
    w.push_back("giap");
    w.push_back("giat");
    w.push_back("giau");
    w.push_back("giauh");
    w.push_back("gih");
    w.push_back("gim");
    w.push_back("gin");
    w.push_back("gio");
    w.push_back("gioh");
    w.push_back("giok");
    w.push_back("giong");
    w.push_back("gip");
    w.push_back("git");
    w.push_back("giu");
    w.push_back("giuh");
    w.push_back("go");
    w.push_back("goa");
    w.push_back("goan");
    w.push_back("goat");
    w.push_back("goe");
    w.push_back("goeh");
    w.push_back("goh");
    w.push_back("gok");
    w.push_back("gong");
    w.push_back("gop");
    w.push_back("gor");
    w.push_back("gorh");
    w.push_back("gou");
    w.push_back("gouh");
    w.push_back("gu");
    w.push_back("guh");
    w.push_back("gui");
    w.push_back("gun");
    w.push_back("gur");
    w.push_back("gurn");
    w.push_back("ha");
    w.push_back("hah");
    w.push_back("hahnn");
    w.push_back("hai");
    w.push_back("hainn");
    w.push_back("hak");
    w.push_back("ham");
    w.push_back("han");
    w.push_back("hang");
    w.push_back("hann");
    w.push_back("hap");
    w.push_back("hat");
    w.push_back("hau");
    w.push_back("hauh");
    w.push_back("hauhnn");
    w.push_back("haunn");
    w.push_back("he");
    w.push_back("heh");
    w.push_back("hehnn");
    w.push_back("hek");
    w.push_back("hem");
    w.push_back("heng");
    w.push_back("henn");
    w.push_back("hi");
    w.push_back("hia");
    w.push_back("hiaan");
    w.push_back("hiah");
    w.push_back("hiahnn");
    w.push_back("hiai");
    w.push_back("hiam");
    w.push_back("hian");
    w.push_back("hiang");
    w.push_back("hiann");
    w.push_back("hiap");
    w.push_back("hiat");
    w.push_back("hiau");
    w.push_back("hiauh");
    w.push_back("hiaunn");
    w.push_back("hih");
    w.push_back("hihnn");
    w.push_back("him");
    w.push_back("hin");
    w.push_back("hinn");
    w.push_back("hio");
    w.push_back("hioh");
    w.push_back("hiong");
    w.push_back("hiou");
    w.push_back("hip");
    w.push_back("hit");
    w.push_back("hiu");
    w.push_back("hiuh");
    w.push_back("hiuhnn");
    w.push_back("hiunn");
    w.push_back("hiut");
    w.push_back("hm");
    w.push_back("hmh");
    w.push_back("hng");
    w.push_back("hngh");
    w.push_back("ho");
    w.push_back("hoa");
    w.push_back("hoah");
    w.push_back("hoai");
    w.push_back("hoaih");
    w.push_back("hoainn");
    w.push_back("hoan");
    w.push_back("hoang");
    w.push_back("hoann");
    w.push_back("hoat");
    w.push_back("hoe");
    w.push_back("hoeh");
    w.push_back("hoh");
    w.push_back("hohnn");
    w.push_back("hok");
    w.push_back("hom");
    w.push_back("hong");
    w.push_back("honn");
    w.push_back("hop");
    w.push_back("hor");
    w.push_back("hou");
    w.push_back("houh");
    w.push_back("houhnn");
    w.push_back("hounn");
    w.push_back("hpohnn");
    w.push_back("hu");
    w.push_back("huh");
    w.push_back("hui");
    w.push_back("huih");
    w.push_back("huinn");
    w.push_back("hun");
    w.push_back("hur");
    w.push_back("hurn");
    w.push_back("hut");
    w.push_back("huuih");
    w.push_back("i");
    w.push_back("ia");
    w.push_back("iah");
    w.push_back("iak");
    w.push_back("iam");
    w.push_back("ian");
    w.push_back("iang");
    w.push_back("iann");
    w.push_back("iap");
    w.push_back("iat");
    w.push_back("iau");
    w.push_back("iaunn");
    w.push_back("ih");
    w.push_back("ihnn");
    w.push_back("im");
    w.push_back("in");
    w.push_back("inn");
    w.push_back("io");
    w.push_back("ioh");
    w.push_back("iok");
    w.push_back("iong");
    w.push_back("ionn");
    w.push_back("iou");
    w.push_back("ip");
    w.push_back("it");
    w.push_back("iu");
    w.push_back("iunn");
    w.push_back("je");
    w.push_back("jeh");
    w.push_back("jek");
    w.push_back("jem");
    w.push_back("jeng");
    w.push_back("ji");
    w.push_back("jia");
    w.push_back("jiah");
    w.push_back("jiak");
    w.push_back("jiam");
    w.push_back("jian");
    w.push_back("jiang");
    w.push_back("jiap");
    w.push_back("jiat");
    w.push_back("jiau");
    w.push_back("jih");
    w.push_back("jim");
    w.push_back("jin");
    w.push_back("jio");
    w.push_back("jioh");
    w.push_back("jiok");
    w.push_back("jiong");
    w.push_back("jiou");
    w.push_back("jip");
    w.push_back("jit");
    w.push_back("jiu");
    w.push_back("joa");
    w.push_back("joah");
    w.push_back("joan");
    w.push_back("joe");
    w.push_back("jor");
    w.push_back("jou");
    w.push_back("ju");
    w.push_back("juh");
    w.push_back("jun");
    w.push_back("jur");
    w.push_back("ka");
    w.push_back("ka");
    w.push_back("kah");
    w.push_back("kai");
    w.push_back("kainn");
    w.push_back("kak");
    w.push_back("kam");
    w.push_back("kan");
    w.push_back("kang");
    w.push_back("kann");
    w.push_back("kao");
    w.push_back("kap");
    w.push_back("kat");
    w.push_back("kau");
    w.push_back("kauh");
    w.push_back("ke");
    w.push_back("keh");
    w.push_back("kehnn");
    w.push_back("kek");
    w.push_back("keng");
    w.push_back("kenn");
    w.push_back("kha");
    w.push_back("khah");
    w.push_back("khai");
    w.push_back("khainn");
    w.push_back("khak");
    w.push_back("kham");
    w.push_back("khan");
    w.push_back("khang");
    w.push_back("khann");
    w.push_back("khap");
    w.push_back("khat");
    w.push_back("khau");
    w.push_back("khauh");
    w.push_back("khauhnn");
    w.push_back("khaunn");
    w.push_back("khe");
    w.push_back("kheh");
    w.push_back("khehnn");
    w.push_back("khek");
    w.push_back("kheng");
    w.push_back("khenn");
    w.push_back("khi");
    w.push_back("khia");
    w.push_back("khiah");
    w.push_back("khiai");
    w.push_back("khiaih");
    w.push_back("khiak");
    w.push_back("khiam");
    w.push_back("khian");
    w.push_back("khiang");
    w.push_back("khiap");
    w.push_back("khiat");
    w.push_back("khiau");
    w.push_back("khiauh");
    w.push_back("khiauhnn");
    w.push_back("khiauk");
    w.push_back("khih");
    w.push_back("khihnn");
    w.push_back("khim");
    w.push_back("khin");
    w.push_back("khing");
    w.push_back("khinn");
    w.push_back("khio");
    w.push_back("khioh");
    w.push_back("khiok");
    w.push_back("khiong");
    w.push_back("khiou");
    w.push_back("khip");
    w.push_back("khit");
    w.push_back("khiu");
    w.push_back("khiunn");
    w.push_back("khng");
    w.push_back("khngh");
    w.push_back("kho");
    w.push_back("khoa");
    w.push_back("khoah");
    w.push_back("khoai");
    w.push_back("khoainn");
    w.push_back("khoan");
    w.push_back("khoang");
    w.push_back("khoann");
    w.push_back("khoat");
    w.push_back("khoe");
    w.push_back("khoeh");
    w.push_back("khoh");
    w.push_back("khok");
    w.push_back("khom");
    w.push_back("khon");
    w.push_back("khong");
    w.push_back("khonn");
    w.push_back("khop");
    w.push_back("khor");
    w.push_back("khorh");
    w.push_back("khou");
    w.push_back("khouh");
    w.push_back("khounn");
    w.push_back("khu");
    w.push_back("khuh");
    w.push_back("khui");
    w.push_back("khuinn");
    w.push_back("khuk");
    w.push_back("khun");
    w.push_back("khur");
    w.push_back("khurh");
    w.push_back("khurn");
    w.push_back("khut");
    w.push_back("ki");
    w.push_back("kia");
    w.push_back("kiah");
    w.push_back("kiak");
    w.push_back("kiam");
    w.push_back("kian");
    w.push_back("kiang");
    w.push_back("kiann");
    w.push_back("kiap");
    w.push_back("kiat");
    w.push_back("kiau");
    w.push_back("kih");
    w.push_back("kihnn");
    w.push_back("kim");
    w.push_back("kin");
    w.push_back("kinn");
    w.push_back("kio");
    w.push_back("kioh");
    w.push_back("kiok");
    w.push_back("kiong");
    w.push_back("kip");
    w.push_back("kit");
    w.push_back("kiu");
    w.push_back("kiuh");
    w.push_back("kiunn");
    w.push_back("kng");
    w.push_back("kngh");
    w.push_back("ko");
    w.push_back("koa");
    w.push_back("koah");
    w.push_back("koai");
    w.push_back("koaihnn");
    w.push_back("koainn");
    w.push_back("koak");
    w.push_back("koamm");
    w.push_back("koan");
    w.push_back("koang");
    w.push_back("koann");
    w.push_back("koat");
    w.push_back("koe");
    w.push_back("koeh");
    w.push_back("koen");
    w.push_back("koenn");
    w.push_back("koh");
    w.push_back("kok");
    w.push_back("kong");
    w.push_back("konn");
    w.push_back("kop");
    w.push_back("kor");
    w.push_back("kou");
    w.push_back("kouh");
    w.push_back("kounn");
    w.push_back("ku");
    w.push_back("kuh");
    w.push_back("kui");
    w.push_back("kuih");
    w.push_back("kuinn");
    w.push_back("kun");
    w.push_back("kur");
    w.push_back("kurn");
    w.push_back("kut");
    w.push_back("la");
    w.push_back("lah");
    w.push_back("lai");
    w.push_back("laih");
    w.push_back("lain");
    w.push_back("lak");
    w.push_back("lam");
    w.push_back("lan");
    w.push_back("lang");
    w.push_back("lap");
    w.push_back("lat");
    w.push_back("lau");
    w.push_back("lauh");
    w.push_back("le");
    w.push_back("leh");
    w.push_back("lek");
    w.push_back("leng");
    w.push_back("leo");
    w.push_back("let");
    w.push_back("li");
    w.push_back("lia");
    w.push_back("liah");
    w.push_back("liak");
    w.push_back("liam");
    w.push_back("lian");
    w.push_back("liang");
    w.push_back("liap");
    w.push_back("liat");
    w.push_back("liau");
    w.push_back("lih");
    w.push_back("lim");
    w.push_back("lin");
    w.push_back("lio");
    w.push_back("lioh");
    w.push_back("liok");
    w.push_back("liong");
    w.push_back("liou");
    w.push_back("liouh");
    w.push_back("lip");
    w.push_back("lit");
    w.push_back("liu");
    w.push_back("liuh");
    w.push_back("lo");
    w.push_back("loa");
    w.push_back("loah");
    w.push_back("loai");
    w.push_back("loaih");
    w.push_back("loan");
    w.push_back("loat");
    w.push_back("loe");
    w.push_back("loeh");
    w.push_back("loh");
    w.push_back("lok");
    w.push_back("lom");
    w.push_back("long");
    w.push_back("lop");
    w.push_back("lor");
    w.push_back("lou");
    w.push_back("louh");
    w.push_back("lu");
    w.push_back("luh");
    w.push_back("lui");
    w.push_back("lun");
    w.push_back("lur");
    w.push_back("lut");
    w.push_back("m");
    w.push_back("ma");
    w.push_back("mah");
    w.push_back("mai");
    w.push_back("mak");
    w.push_back("mam");
    w.push_back("man");
    w.push_back("mang");
    w.push_back("mat");
    w.push_back("mau");
    w.push_back("mauh");
    w.push_back("me");
    w.push_back("meh");
    w.push_back("mek");
    w.push_back("mem");
    w.push_back("men");
    w.push_back("menn");
    w.push_back("mh");
    w.push_back("mi");
    w.push_back("mia");
    w.push_back("mian");
    w.push_back("miann");
    w.push_back("miau");
    w.push_back("mih");
    w.push_back("min");
    w.push_back("minn");
    w.push_back("mng");
    w.push_back("mngh");
    w.push_back("moa");
    w.push_back("moai");
    w.push_back("moann");
    w.push_back("moau");
    w.push_back("moe");
    w.push_back("moh");
    w.push_back("mong");
    w.push_back("mou");
    w.push_back("mouh");
    w.push_back("mounn");
    w.push_back("mu");
    w.push_back("muh");
    w.push_back("mui");
    w.push_back("muih");
    w.push_back("muinn");
    w.push_back("na");
    w.push_back("nah");
    w.push_back("nai");
    w.push_back("naih");
    w.push_back("nam");
    w.push_back("nat");
    w.push_back("nau");
    w.push_back("nauh");
    w.push_back("ne");
    w.push_back("neh");
    w.push_back("nep");
    w.push_back("net");
    w.push_back("ng");
    w.push_back("nga");
    w.push_back("ngai");
    w.push_back("ngau");
    w.push_back("ngauh");
    w.push_back("nge");
    w.push_back("ngeh");
    w.push_back("ngh");
    w.push_back("ngi");
    w.push_back("ngia");
    w.push_back("ngiau");
    w.push_back("ngiauh");
    w.push_back("ngih");
    w.push_back("ngiu");
    w.push_back("ngiuh");
    w.push_back("ngoeh");
    w.push_back("ngou");
    w.push_back("ngui");
    w.push_back("ni");
    w.push_back("nia");
    w.push_back("niau");
    w.push_back("niauh");
    w.push_back("nih");
    w.push_back("nin");
    w.push_back("niu");
    w.push_back("niuh");
    w.push_back("nng");
    w.push_back("noa");
    w.push_back("noai");
    w.push_back("noann");
    w.push_back("nou");
    w.push_back("nouh");
    w.push_back("nui");
    w.push_back("nuinn");
    w.push_back("nun");
    w.push_back("o");
    w.push_back("oa");
    w.push_back("oah");
    w.push_back("oai");
    w.push_back("oaih");
    w.push_back("oaihnn");
    w.push_back("oainn");
    w.push_back("oak");
    w.push_back("oan");
    w.push_back("oang");
    w.push_back("oann");
    w.push_back("oat");
    w.push_back("oe");
    w.push_back("oeh");
    w.push_back("oh");
    w.push_back("ohnn");
    w.push_back("ok");
    w.push_back("om");
    w.push_back("ong");
    w.push_back("onn");
    w.push_back("op");
    w.push_back("or");
    w.push_back("orh");
    w.push_back("ou");
    w.push_back("ouh");
    w.push_back("ouhnn");
    w.push_back("ounn");
    w.push_back("pa");
    w.push_back("pah");
    w.push_back("pai");
    w.push_back("pain");
    w.push_back("painn");
    w.push_back("pak");
    w.push_back("pan");
    w.push_back("pang");
    w.push_back("pann");
    w.push_back("pat");
    w.push_back("pau");
    w.push_back("pauh");
    w.push_back("pe");
    w.push_back("peh");
    w.push_back("pek");
    w.push_back("peng");
    w.push_back("penn");
    w.push_back("pgainn");
    w.push_back("pgang");
    w.push_back("pha");
    w.push_back("phah");
    w.push_back("phai");
    w.push_back("phainn");
    w.push_back("phak");
    w.push_back("phan");
    w.push_back("phang");
    w.push_back("phann");
    w.push_back("phat");
    w.push_back("phau");
    w.push_back("phauh");
    w.push_back("phe");
    w.push_back("pheh");
    w.push_back("phek");
    w.push_back("pheng");
    w.push_back("phenn");
    w.push_back("phep");
    w.push_back("phi");
    w.push_back("phia");
    w.push_back("phiah");
    w.push_back("phiak");
    w.push_back("phian");
    w.push_back("phiang");
    w.push_back("phiann");
    w.push_back("phiat");
    w.push_back("phiau");
    w.push_back("phih");
    w.push_back("phin");
    w.push_back("phinn");
    w.push_back("phio");
    w.push_back("phiou");
    w.push_back("phit");
    w.push_back("phiu");
    w.push_back("phng");
    w.push_back("phngh");
    w.push_back("pho");
    w.push_back("phoa");
    w.push_back("phoah");
    w.push_back("phoann");
    w.push_back("phoat");
    w.push_back("phoe");
    w.push_back("phoeh");
    w.push_back("phoh");
    w.push_back("phok");
    w.push_back("phong");
    w.push_back("phor");
    w.push_back("phorh");
    w.push_back("phou");
    w.push_back("phu");
    w.push_back("phuh");
    w.push_back("phui");
    w.push_back("phun");
    w.push_back("phut");
    w.push_back("pi");
    w.push_back("pia");
    w.push_back("piah");
    w.push_back("piak");
    w.push_back("pian");
    w.push_back("piang");
    w.push_back("piann");
    w.push_back("piat");
    w.push_back("piau");
    w.push_back("pih");
    w.push_back("pin");
    w.push_back("pinn");
    w.push_back("pio");
    w.push_back("pit");
    w.push_back("piu");
    w.push_back("pkoe");
    w.push_back("png");
    w.push_back("po");
    w.push_back("poa");
    w.push_back("poah");
    w.push_back("poan");
    w.push_back("poann");
    w.push_back("poat");
    w.push_back("poe");
    w.push_back("poeh");
    w.push_back("poh");
    w.push_back("poinn");
    w.push_back("pok");
    w.push_back("pon");
    w.push_back("pong");
    w.push_back("ponn");
    w.push_back("por");
    w.push_back("porh");
    w.push_back("pou");
    w.push_back("pounn");
    w.push_back("pu");
    w.push_back("puh");
    w.push_back("pui");
    w.push_back("puih");
    w.push_back("puinn");
    w.push_back("pun");
    w.push_back("put");
    w.push_back("sa");
    w.push_back("sah");
    w.push_back("sahnn");
    w.push_back("sai");
    w.push_back("saihnn");
    w.push_back("sainn");
    w.push_back("sak");
    w.push_back("sam");
    w.push_back("san");
    w.push_back("sang");
    w.push_back("sann");
    w.push_back("sap");
    w.push_back("sat");
    w.push_back("sau");
    w.push_back("sauhnn");
    w.push_back("se");
    w.push_back("seh");
    w.push_back("sek");
    w.push_back("sem");
    w.push_back("seng");
    w.push_back("senn");
    w.push_back("shiong");
    w.push_back("si");
    w.push_back("sia");
    w.push_back("siah");
    w.push_back("siahnn");
    w.push_back("siak");
    w.push_back("siam");
    w.push_back("sian");
    w.push_back("siang");
    w.push_back("siann");
    w.push_back("siap");
    w.push_back("siat");
    w.push_back("siau");
    w.push_back("sih");
    w.push_back("sihnn");
    w.push_back("sim");
    w.push_back("sin");
    w.push_back("sing");
    w.push_back("sinn");
    w.push_back("sio");
    w.push_back("sioh");
    w.push_back("siok");
    w.push_back("siong");
    w.push_back("siot");
    w.push_back("siou");
    w.push_back("siouh");
    w.push_back("sip");
    w.push_back("sit");
    w.push_back("siu");
    w.push_back("siuh");
    w.push_back("siunn");
    w.push_back("siut");
    w.push_back("sng");
    w.push_back("sngh");
    w.push_back("so");
    w.push_back("soa");
    w.push_back("soah");
    w.push_back("soaihnn");
    w.push_back("soainn");
    w.push_back("soan");
    w.push_back("soann");
    w.push_back("soat");
    w.push_back("soe");
    w.push_back("soeh");
    w.push_back("soh");
    w.push_back("sok");
    w.push_back("som");
    w.push_back("song");
    w.push_back("sop");
    w.push_back("sor");
    w.push_back("sorh");
    w.push_back("sou");
    w.push_back("souh");
    w.push_back("su");
    w.push_back("suh");
    w.push_back("sui");
    w.push_back("suinn");
    w.push_back("sun");
    w.push_back("sur");
    w.push_back("surh");
    w.push_back("sut");
    w.push_back("ta");
    w.push_back("tah");
    w.push_back("tai");
    w.push_back("taih");
    w.push_back("tainn");
    w.push_back("tak");
    w.push_back("tam");
    w.push_back("tan");
    w.push_back("tang");
    w.push_back("tann");
    w.push_back("tap");
    w.push_back("tat");
    w.push_back("tau");
    w.push_back("tauh");
    w.push_back("te");
    w.push_back("teh");
    w.push_back("tek");
    w.push_back("teng");
    w.push_back("tenn");
    w.push_back("tha");
    w.push_back("thah");
    w.push_back("thai");
    w.push_back("thainn");
    w.push_back("thak");
    w.push_back("tham");
    w.push_back("than");
    w.push_back("thang");
    w.push_back("thann");
    w.push_back("thap");
    w.push_back("that");
    w.push_back("thau");
    w.push_back("the");
    w.push_back("theh");
    w.push_back("theik");
    w.push_back("thek");
    w.push_back("them");
    w.push_back("theng");
    w.push_back("thenn");
    w.push_back("thi");
    w.push_back("thia");
    w.push_back("thiah");
    w.push_back("thiam");
    w.push_back("thian");
    w.push_back("thiann");
    w.push_back("thiap");
    w.push_back("thiat");
    w.push_back("thiau");
    w.push_back("thih");
    w.push_back("thim");
    w.push_back("thin");
    w.push_back("thing");
    w.push_back("thinn");
    w.push_back("thio");
    w.push_back("thiok");
    w.push_back("thiong");
    w.push_back("thip");
    w.push_back("thit");
    w.push_back("thiu");
    w.push_back("thiunn");
    w.push_back("thng");
    w.push_back("tho");
    w.push_back("thoa");
    w.push_back("thoah");
    w.push_back("thoan");
    w.push_back("thoann");
    w.push_back("thoat");
    w.push_back("thoe");
    w.push_back("thoeh");
    w.push_back("thoh");
    w.push_back("thok");
    w.push_back("thom");
    w.push_back("thong");
    w.push_back("thor");
    w.push_back("thou");
    w.push_back("thu");
    w.push_back("thuh");
    w.push_back("thui");
    w.push_back("thuinn");
    w.push_back("thun");
    w.push_back("thut");
    w.push_back("ti");
    w.push_back("tiah");
    w.push_back("tiak");
    w.push_back("tiam");
    w.push_back("tian");
    w.push_back("tiang");
    w.push_back("tiann");
    w.push_back("tiap");
    w.push_back("tiat");
    w.push_back("tiau");
    w.push_back("tiauh");
    w.push_back("tiauhnn");
    w.push_back("tiaunn");
    w.push_back("tih");
    w.push_back("tihnn");
    w.push_back("tik");
    w.push_back("tim");
    w.push_back("tin");
    w.push_back("tinn");
    w.push_back("tio");
    w.push_back("tioh");
    w.push_back("tioing");
    w.push_back("tiok");
    w.push_back("tiong");
    w.push_back("tit");
    w.push_back("tiu");
    w.push_back("tiuh");
    w.push_back("tiunn");
    w.push_back("tng");
    w.push_back("to");
    w.push_back("toa");
    w.push_back("toah");
    w.push_back("toainn");
    w.push_back("toam");
    w.push_back("toan");
    w.push_back("toann");
    w.push_back("toe");
    w.push_back("toeh");
    w.push_back("toh");
    w.push_back("tok");
    w.push_back("tom");
    w.push_back("tong");
    w.push_back("tonn");
    w.push_back("top");
    w.push_back("tor");
    w.push_back("torh");
    w.push_back("tou");
    w.push_back("touh");
    w.push_back("tu");
    w.push_back("tua");
    w.push_back("tuh");
    w.push_back("tui");
    w.push_back("tuinn");
    w.push_back("tun");
    w.push_back("tur");
    w.push_back("turn");
    w.push_back("tut");
    w.push_back("u");
    w.push_back("ua");
    w.push_back("uh");
    w.push_back("ui");
    w.push_back("uih");
    w.push_back("uinn");
    w.push_back("uit");
    w.push_back("un");
    w.push_back("unn");
    w.push_back("uong");
    w.push_back("ur");
    w.push_back("urn");
    w.push_back("ut");
    w.push_back("utu");
    w.push_back("va");
    w.push_back("vi");
    w.push_back("vui");
    w.push_back("vurn");

    ins(w);
    std::vector<std::string> res = trie->autocomplete(u8"a");
    BOOST_TEST((std::find(res.begin(), res.end(), u8"ang") != res.end()));
    BOOST_TEST((std::find(res.begin(), res.end(), u8"any") == res.end()));
}

BOOST_AUTO_TEST_SUITE_END();
