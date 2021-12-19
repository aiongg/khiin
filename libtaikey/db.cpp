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

void TKDB::init() {
    bool hasTrieMapTable = false;

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

std::vector<std::string> TKDB::selectTrieWordlist() {
    SQLite::Statement query(db_, "select distinct ascii from trie_map");
    std::vector<std::string> res;

    while (query.executeStep()) {
        res.push_back(query.getColumn("ascii").getString());
    }

    return res;
}

std::vector<std::string> TKDB::selectSyllableList() {
    SQLite::Statement query(db_, "select distinct syl from syllables_by_freq order by id asc");
    std::vector<std::string> res;

    while (query.executeStep()) {
        res.push_back(query.getColumn("syl").getString());
    }

    return res;
}

std::vector<DictionaryRow>
TKDB::selectDictionaryRowsByAscii(std::string ascii) {
    std::vector<DictionaryRow> res;

    SQLite::Statement query(
        db_, "select dictionary.* from trie_map inner join dictionary on "
             "trie_map.dictionary_id = dictionary.id where trie_map.ascii = ?");
    query.bind(1, ascii);

    while (query.executeStep()) {
        DictionaryRow d;
        d.id = query.getColumn("id").getInt();
        d.chhan_id = query.getColumn("chhan_id").getInt();
        d.lomaji = query.getColumn("lomaji").getString();
        d.hanji = query.getColumn("hanji").getString();
        d.weight = query.getColumn("weight").getInt();
        d.common = query.getColumn("common").getInt();
        d.hint = query.getColumn("hint").getString();
        res.push_back(d);
    }

    return res;
}

int TKDB::buildTrieLookupTable_() {
    std::vector<std::string> insertions;

    SQLite::Statement dictionaryQuery(db_, "select * from dictionary");

    while (dictionaryQuery.executeStep()) {
        int dict_id = dictionaryQuery.getColumn("id").getInt();
        std::string asciiLomaji =
            utf8ToAsciiLower(dictionaryQuery.getColumn("lomaji").getString());

        static boost::regex rSylSep("[ -]+");
        std::string collapsed = boost::regex_replace(asciiLomaji, rSylSep, "");
        static boost::regex rInnerTone("\\d(?!$)");
        std::string toneless = boost::regex_replace(collapsed, rInnerTone, "");

        insertions.push_back("('" + collapsed + "', " +
                             std::to_string(dict_id) + ")");

        if (collapsed != toneless) {
            insertions.push_back("('" + toneless + "', " +
                                 std::to_string(dict_id) + ")");
        }
    }

    std::string insertQuery =
        "insert into trie_map (ascii, dictionary_id) values " +
        boost::algorithm::join(insertions, ", ");

    SQLite::Transaction tx(db_);

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
