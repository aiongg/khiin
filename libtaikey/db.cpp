#include <unordered_set>

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/regex.hpp>

#include "db.h"
#include "lomaji.h"
#include "sql.h"

namespace TaiKey {

using namespace std::literals::string_literals;

// Testing only
TKDB::TKDB() : db_(SQLite::Database(":memory:", SQLite::OPEN_READWRITE)) {}

TKDB::TKDB(std::string dbFilename)
    : db_(SQLite::Database(dbFilename, SQLite::OPEN_READWRITE)) {
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
                        query.getColumn("color").getInt(),
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
                        query.getColumn("color").getInt(),
                        query.getColumn("hint").getString()};
        results.push_back(d);
    }
}

auto TKDB::selectCandidatesFor(VStr inputs, Candidates &results) -> void {
    results.clear();

    auto query = SQLite::Statement(
        db_, SQL::SELECT_DictionaryWithUnigrams(inputs.size()));

    for (const auto &in : inputs | boost::adaptors::indexed(1)) {
        query.bind(static_cast<int>(in.index()), in.value());
    }

    while (query.executeStep()) {
        Candidate d{query.getColumn("id").getInt(),
                    query.getColumn("ascii").getString(),
                    query.getColumn("input").getString(),
                    query.getColumn("output").getString(),
                    query.getColumn("hint").getString(),
                    query.getColumn("color").getInt(),
                    query.getColumn("unigram_n").getInt()};
        results.push_back(d);
    }
}

auto TKDB::getUnigramCount(std::string gram) -> int {
    if (gram.empty()) {
        return 0;
    }

    auto query = SQLite::Statement(db_, SQL::SELECT_Unigram);
    query.bind(1, gram);
    auto found = query.executeStep();

    return found ? query.getColumn("n").getInt() : 0;
}

auto TKDB::selectBigramsFor(std::string lgram, VStr rgrams,
                            BigramWeights &results) -> void {
    results.clear();

    auto query = SQLite::Statement(db_, SQL::SELECT_Bigrams(rgrams.size()));

    query.bind(1, lgram);

    for (const auto &r : rgrams | boost::adaptors::indexed(2)) {
        query.bind(static_cast<int>(r.index()), r.value());
    }

    while (query.executeStep()) {
        results[query.getColumn("rgram").getString()] =
            query.getColumn("n").getInt();
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

    // BOOST_LOG_TRIVIAL(debug) << rawUnigrams;
    // BOOST_LOG_TRIVIAL(debug) << rawBigrams;

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

    auto tx = SQLite::Transaction(db_);

    db_.exec(SQL::CREATE_TrieMapTable);

    auto dictionaryQuery = SQLite::Statement(db_, SQL::SELECT_DictionaryInputs);

    auto insertQueries = std::vector<SQLite::Statement>();

    while (dictionaryQuery.executeStep()) {
        auto dictId = dictionaryQuery.getColumn("id").getInt();
        auto ascii =
            utf8ToAsciiLower(dictionaryQuery.getColumn("input").getString());
        auto output = dictionaryQuery.getColumn("output").getString();

        auto needsHyphens =
            std::find(output.cbegin(), output.cend(), '-') != output.cend();

        static boost::regex rInnerTone("\\d(?!$)");

        boost::algorithm::replace_all(ascii, " ", "");
        auto collapsed = boost::replace_all_copy(ascii, "-", "");
        auto collapsedNoTone = boost::regex_replace(collapsed, rInnerTone, "");

        auto uniques = std::unordered_set<std::string>();
        if (needsHyphens) {
            uniques.insert(ascii);
            uniques.insert(boost::regex_replace(ascii, rInnerTone, ""));
        }
        uniques.insert(collapsed);
        uniques.insert(collapsedNoTone);

        for (const auto &ea : uniques) {
            insertable.push_back(std::make_pair(ea, dictId));
        }

        if (insertable.size() > 1000) {
            insertQueries.emplace_back(
                SQLite::Statement(db_, SQL::INSERT_TrieMap(insertable.size())));
            for (const auto &each : insertable | boost::adaptors::indexed(1)) {
                insertQueries.back().bind(
                    static_cast<int>(2 * each.index() - 1), each.value().first);
                insertQueries.back().bind(static_cast<int>(2 * each.index()),
                                          each.value().second);
            }
            insertable.clear();
        }
    }

    insertQueries.emplace_back(
        SQLite::Statement(db_, SQL::INSERT_TrieMap(insertable.size())));
    for (const auto &each : insertable | boost::adaptors::indexed(1)) {
        insertQueries.back().bind(static_cast<int>(2 * each.index() - 1),
                                  each.value().first);
        insertQueries.back().bind(static_cast<int>(2 * each.index()),
                                  each.value().second);
    }

    for (auto &q : insertQueries) {
        q.exec();
    }

    db_.exec(SQL::INDEX_TrieMapTable);

    try {
        tx.commit();
        return 0;
    } catch (std::exception &e) {
        BOOST_LOG_TRIVIAL(debug) << e.what();
        return -1;
    }
}

} // namespace TaiKey
