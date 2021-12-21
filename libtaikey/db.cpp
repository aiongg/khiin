#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include "db.h"
#include "lomaji.h"

namespace TaiKey {

TKDB::TKDB(std::string dbFilename)
    : db_(SQLite::Database(dbFilename,
                           SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)) {
    tableDictionary_.reserve(20000);
    init();
}

auto TKDB::init() -> void {
    auto hasTrieMapTable = false;

    try {
        db_.execAndGet("select name from sqlite_master where "
                       "type='table' and name='trie_map'");
        hasTrieMapTable = true;
    } catch (...) {
        // Do nothing, leave the boolean false
    }

    if (!hasTrieMapTable) {
        buildTrieLookupTable_();
    }
}

auto TKDB::selectTrieWordlist() -> VStr {
    auto query = SQLite::Statement(db_, "select distinct ascii from trie_map");
    VStr res;

    while (query.executeStep()) {
        res.push_back(query.getColumn("ascii").getString());
    }

    return res;
}

auto TKDB::selectSyllableList() -> VStr {
    SQLite::Statement query(
        db_, "select distinct syl from syllables_by_freq order by id asc");
    VStr res;

    while (query.executeStep()) {
        res.push_back(query.getColumn("syl").getString());
    }

    return res;
}

auto TKDB::selectDictionaryRowsByAscii(std::string ascii) -> DictRowV {
    DictRowV res;

    auto query = SQLite::Statement(
        db_, "select dictionary.* from trie_map inner join dictionary on "
             "trie_map.dictionary_id = dictionary.id where trie_map.ascii = ?");
    query.bind(1, ascii);

    while (query.executeStep()) {
        DictionaryRow d{query.getColumn("id").getInt(),
                        query.getColumn("chhan_id").getInt(),
                        query.getColumn("input").getString(),
                        query.getColumn("output").getString(),
                        query.getColumn("weight").getInt(),
                        query.getColumn("common").getInt(),
                        query.getColumn("hint").getString()};
        res.push_back(d);
    }

    return res;
}

int TKDB::buildTrieLookupTable_() {
    VStr insertions;

    auto dictionaryQuery = SQLite::Statement(db_, "select * from dictionary");

    while (dictionaryQuery.executeStep()) {
        auto dict_id = dictionaryQuery.getColumn("id").getInt();
        auto asciiLomaji =
            utf8ToAsciiLower(dictionaryQuery.getColumn("input").getString());

        static boost::regex rSylSep("[ -]+");
        auto collapsed = boost::regex_replace(asciiLomaji, rSylSep, "");
        static boost::regex rInnerTone("\\d(?!$)");
        auto toneless = boost::regex_replace(collapsed, rInnerTone, "");

        insertions.push_back("('" + collapsed + "', " +
                             std::to_string(dict_id) + ")");

        if (collapsed != toneless) {
            insertions.push_back("('" + toneless + "', " +
                                 std::to_string(dict_id) + ")");
        }
    }

    auto insertQuery =
        "insert into trie_map (ascii, dictionary_id) values " +
        boost::algorithm::join(insertions, ", ");

    auto tx = SQLite::Transaction(db_);

    db_.exec("drop table if exists trie_map; "
             "create table trie_map (id integer primary key, ascii text, "
             "dictionary_id id, "
             "foreign key (dictionary_id) references dictionary (id), "
             "unique(ascii, dictionary_id))");
    db_.exec(insertQuery);
    db_.exec("create index trie_map_ascii_idx on trie_map (ascii)");

    try {
        tx.commit();
        return 0;
    } catch (std::exception &e) {
        return -1;
    }
}

} // namespace TaiKey
