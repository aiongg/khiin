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

    for (const auto &in : inputs | boost::adaptors::indexed(1)) {
        query.bind(static_cast<int>(in.index()), in.value());
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

auto TKDB::selectDictionaryRowsByAsciiWithUnigram(VStr inputs,
                                                  DictRows &results) -> void {
    results.clear();

    auto query = SQLite::Statement(
        db_, SQL::SELECT_DictionaryWithUnigrams(inputs.size()));

    for (const auto &in : inputs | boost::adaptors::indexed(1)) {
        query.bind(static_cast<int>(in.index()), in.value());
    }

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
        auto oneGramQuery = SQLite::Statement(db_, SQL::UPSERT_OneGram);
        oneGramQuery.bind(1, grams[0]);
        oneGramQuery.bind(2, grams[0]);
        return oneGramQuery.exec();
    }

    // For a list of grams > 1, we need both uni- and bigram

    // auto nUnigrams = grams.size();
    // auto nBigrams = grams.size() + 1;
    auto nBigrams = grams.size() - 1;

    auto rawUnigrams = SQL::UPSERT_Unigrams(grams.size());
    auto rawBigrams = SQL::UPSERT_Bigrams(nBigrams);

    BOOST_LOG_TRIVIAL(debug) << rawUnigrams;
    BOOST_LOG_TRIVIAL(debug) << rawBigrams;

    auto qUnigrams = SQLite::Statement(db_, rawUnigrams);

    for (const auto &gram : grams | boost::adaptors::indexed(1)) {
        qUnigrams.bind(static_cast<int>(gram.index()), gram.value());
        qUnigrams.bind(static_cast<int>(gram.index() + grams.size()),
                       gram.value());
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
        qBigrams.bind(static_cast<int>(i + 2 * nBigrams), *it);
        qBigrams.bind(++i, *(++it));
        qBigrams.bind(static_cast<int>(i + 2 * nBigrams), *it);
    }

    auto res = qUnigrams.exec();
    auto res2 = qBigrams.exec();

    return res + res2;
}

int TKDB::buildTrieLookupTable_() {
    std::vector<std::pair<std::string, int>> insertable;

    auto dictionaryQuery = SQLite::Statement(db_, SQL::SELECT_DictionaryInputs);

    while (dictionaryQuery.executeStep()) {
        auto dictId = dictionaryQuery.getColumn("id").getInt();
        auto asciiLomaji =
            utf8ToAsciiLower(dictionaryQuery.getColumn("input").getString());

        static boost::regex rSylSep("[ -]+");
        auto collapsed = boost::regex_replace(asciiLomaji, rSylSep, "");
        static boost::regex rInnerTone("\\d(?!$)");
        auto toneless = boost::regex_replace(collapsed, rInnerTone, "");

        insertable.push_back(std::make_pair(collapsed, dictId));

        if (collapsed != toneless) {
            insertable.push_back(std::make_pair(toneless, dictId));
        }
    }

    auto tx = SQLite::Transaction(db_);

    db_.exec(SQL::CREATE_TrieMapTable);

    auto insertQuery =
        SQLite::Statement(db_, SQL::INSERT_TrieMap(insertable.size()));
    for (const auto &each : insertable | boost::adaptors::indexed(1)) {
        insertQuery.bind(static_cast<int>(2 * each.index() - 1),
                         each.value().first);
        insertQuery.bind(static_cast<int>(2 * each.index()),
                         each.value().second);
    }

    insertQuery.exec();
    db_.exec(SQL::INDEX_TrieMapTable);

    try {
        tx.commit();
        return 0;
    } catch (std::exception &e) {
        return -1;
    }
}

} // namespace TaiKey
