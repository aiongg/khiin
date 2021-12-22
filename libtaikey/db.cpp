#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/regex.hpp>

#include "db.h"
#include "lomaji.h"
#include "sql.h"

namespace TaiKey {

using namespace std::literals::string_literals;

TKDB::TKDB(std::string dbFilename)
    : db_(SQLite::Database(dbFilename,
                           SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)) {
    tableDictionary_.reserve(20000);
    init();
}

auto TKDB::init() -> void {
    if (!db_.tableExists("trie_map")) {
        buildTrieLookupTable_();
    }
}

auto TKDB::selectTrieWordlist() -> VStr {
    auto query = SQLite::Statement(db_, SQL::SELECT_TrieWords);
    VStr res;

    while (query.executeStep()) {
        res.push_back(query.getColumn("ascii").getString());
    }

    return res;
}

auto TKDB::selectSyllableList() -> VStr {
    SQLite::Statement query(db_, SQL::SELECT_Syllables);
    VStr res;

    while (query.executeStep()) {
        res.push_back(query.getColumn("syl").getString());
    }

    return res;
}

auto TKDB::selectDictionaryRowsByAscii(std::string input, DictRows &results)
    -> void {
    results.clear();

    auto query = SQLite::Statement(db_, SQL::SELECT_Dictionary_1);
    query.bind(1, input);

    while (query.executeStep()) {
        DictionaryRow d{query.getColumn("id").getInt(),
                        query.getColumn("chhan_id").getInt(),
                        query.getColumn("input").getString(),
                        query.getColumn("output").getString(),
                        query.getColumn("weight").getInt(),
                        query.getColumn("common").getInt(),
                        query.getColumn("hint").getString()};
        results.push_back(d);
    }
}

auto TKDB::selectDictionaryRowsByAscii(VStr inputs, DictRows &results) -> void {
    results.clear();

    auto query =
        SQLite::Statement(db_, SQL::SELECT_Dictionary_N(inputs.size()));

    for (const auto &input : inputs | boost::adaptors::indexed(1)) {
        query.bind(input.index(), input.value());
    }

    while (query.executeStep()) {
        DictionaryRow d{query.getColumn("id").getInt(),
                        query.getColumn("chhan_id").getInt(),
                        query.getColumn("input").getString(),
                        query.getColumn("output").getString(),
                        query.getColumn("weight").getInt(),
                        query.getColumn("common").getInt(),
                        query.getColumn("hint").getString()};
        results.push_back(d);
    }
}

auto TKDB::selectDictionaryRowsByAsciiWithUnigram(VStr queries,
                                                  DictRows &results) -> void {
    results.clear();

    auto qstring = "(\"" + boost::algorithm::join(queries, "\",\"") + "\") ";
    auto query = SQLite::Statement(
        db_, "WITH d AS ( "
             "SELECT dictionary.* FROM trie_map INNER JOIN dictionary "
             "ON trie_map.dictionary_id = dictionary.id "
             "WHERE trie_map.ascii IN " +
                 qstring +
                 "AND dictionary.weight >= 1000) "
                 "SELECT d.*, "
                 "CASE WHEN unigram_freq.n IS NULL THEN 0 ELSE unigram_freq.n "
                 "END AS unigram_n "
                 "FROM d LEFT JOIN unigram_freq "
                 "ON d.output = unigram_freq.gram "
                 "ORDER BY unigram_freq.n DESC, "
                 "length(d.input) DESC, "
                 "d.weight DESC, "
                 "d.chhan_id ASC");

    while (query.executeStep()) {
        DictionaryRow d{query.getColumn("id").getInt(),
                        query.getColumn("chhan_id").getInt(),
                        query.getColumn("input").getString(),
                        query.getColumn("output").getString(),
                        query.getColumn("weight").getInt(),
                        query.getColumn("common").getInt(),
                        query.getColumn("hint").getString(),
                        query.getColumn("unigram_n").getInt()};
        results.push_back(d);
    }
}

auto TKDB::updateGramCounts(VStr &grams) -> int {
    // This version builds the strings manually, for testing
    // speed against the ones using .bind below...
    // on average it seems the .bind ones run slightly faster

    // auto uniInsert = std::string();
    // auto uniUpdate = std::string();
    // auto biInsert = std::string();
    // auto biUpdate = std::string();

    // for (auto it = grams.cbegin(); it != grams.cend(); it++) {
    //    uniInsert += "(\"" + *it + "\", 1)";
    //    uniUpdate += "\"" + *it + "\"";

    //    if (it == grams.cbegin()) {
    //        biInsert += "(\"#\", \"" + *it + "\", 1)";
    //        biUpdate += "(\"#\", \"" + *it + "\")";
    //    }

    //    if (std::next(it) == grams.cend()) {
    //        biInsert += ", ";
    //        biUpdate += ", ";
    //        biInsert += "(\"" + *it + "\", \"#\", 1)";
    //        biUpdate += "(\"" + *it + "\", \"#\")";
    //    }

    //    if (std::next(it) != grams.cend()) {
    //        uniInsert += ", ";
    //        uniUpdate += ", ";
    //        biInsert += ", ";
    //        biUpdate += ", ";

    //        biInsert += "(\"" + *it + "\", \"" + *(it + 1) + "\", 1)";
    //        biUpdate += "(\"" + *it + "\", \"" + *(it + 1) + "\")";
    //    }
    //}

    // auto queryUnigrams =
    //    "INSERT INTO unigram_freq (gram, n) VALUES " + uniInsert +
    //    " ON CONFLICT DO UPDATE SET n = n + 1 WHERE gram IN (" + uniUpdate +
    //    ")";

    // auto queryBigrams =
    //    "INSERT INTO bigram_freq (lgram, rgram, n) VALUES " + biInsert +
    //    " ON CONFLICT DO UPDATE SET n = n + 1 WHERE (lgram, rgram) IN ( "
    //    "VALUES " +
    //    biUpdate + " )";

    // BOOST_LOG_TRIVIAL(debug) << queryUnigrams;
    // BOOST_LOG_TRIVIAL(debug) << queryBigrams;

    // auto ret = db_.exec(queryUnigrams);
    // auto ret2 = db_.exec(queryBigrams);

    // return ret + ret2;

    if (grams.size() == 0) {
        return 0;
    }

    if (grams.size() == 1) {
        auto oneGramQuery = SQLite::Statement(
            db_, "INSERT INTO unigram_freq (gram, n) VALUES "
                 "(?, 1) ON CONFLICT DO UPDATE SET n = n + 1 WHERE gram = ?");
        oneGramQuery.bind(1, grams[0]);
        oneGramQuery.bind(2, grams[0]);
        return oneGramQuery.exec();
    }

    // For a list of grams > 1, we need both uni- and bigram

    auto nUnigrams = grams.size();
    // auto nBigrams = grams.size() + 1;
    auto nBigrams = grams.size() - 1;

    auto rawUnigrams = "INSERT INTO unigram_freq (gram, n) VALUES " +
                       boost::algorithm::join(VStr(nUnigrams, "(?, 1)"), ", ") +
                       " ON CONFLICT DO UPDATE SET n = n + 1 WHERE gram IN (" +
                       boost::algorithm::join(VStr(nUnigrams, "?"), ", ") + ")";

    auto rawBigrams =
        "INSERT INTO bigram_freq (lgram, rgram, n) VALUES " +
        boost::algorithm::join(VStr(nBigrams, "(?, ?, 1)"), ", ") +
        " ON CONFLICT DO UPDATE SET n = n + 1 WHERE (lgram, rgram) IN ( "
        "VALUES " +
        boost::algorithm::join(VStr(nBigrams, "(?, ?)"), ", ") + " )";

    BOOST_LOG_TRIVIAL(debug) << rawUnigrams;
    BOOST_LOG_TRIVIAL(debug) << rawBigrams;

    auto qUnigrams = SQLite::Statement(db_, rawUnigrams);

    for (const auto &gram : grams | boost::adaptors::indexed(1)) {
        qUnigrams.bind(gram.index(), gram.value());
        qUnigrams.bind(gram.index() + grams.size(), gram.value());
    }

    auto qBigrams = SQLite::Statement(db_, rawBigrams);

    auto i = 0;
    for (auto it = grams.cbegin(); it != grams.cend();) {
        // if (it == grams.cbegin()) {
        //    qBigrams.bind(++i, "#");
        //    qBigrams.bind(i + 2 * nBigrams, "#");
        //    qBigrams.bind(++i, *it);
        //    qBigrams.bind(i + 2 * nBigrams, *it);
        //}

        if (std::next(it) == grams.cend()) {
            // qBigrams.bind(++i, *it);
            // qBigrams.bind(i + 2 * nBigrams, *it);
            // qBigrams.bind(++i, "#");
            // qBigrams.bind(i + 2 * nBigrams, "#");
            break;
        }

        qBigrams.bind(++i, *it);
        qBigrams.bind(i + 2 * nBigrams, *it);
        qBigrams.bind(++i, *(++it));
        qBigrams.bind(i + 2 * nBigrams, *it);
    }

    auto res = qUnigrams.exec();
    auto res2 = qBigrams.exec();

    return res + res2;
}

int TKDB::buildTrieLookupTable_() {
    VStr insertions;

    auto dictionaryQuery =
        SQLite::Statement(db_, "SELECT id, input FROM dictionary");

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

    auto insertQuery = "INSERT INTO trie_map (ascii, dictionary_id) VALUES " +
                       boost::algorithm::join(insertions, ", ");

    auto tx = SQLite::Transaction(db_);

    db_.exec("DROP TABLE IF EXISTS trie_map; "
             "CREATE TABLE trie_map (id INTEGER PRIMARY KEY, ascii TEXT, "
             "dictionary_id INTEGER, "
             "FOREIGN KEY (dictionary_id) REFERENCES dictionary (id), "
             "UNIQUE(ascii, dictionary_id))");
    db_.exec(insertQuery);
    db_.exec("CREATE INDEX trie_map_ascii_idx ON trie_map (ascii)");

    try {
        tx.commit();
        return 0;
    } catch (std::exception &e) {
        return -1;
    }
}

} // namespace TaiKey
