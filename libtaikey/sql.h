#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "common.h"

namespace TaiKey {
namespace SQL {

using namespace std::literals::string_literals;

static std::string makeSQLBinder(size_t n, std::string sep) {
    std::string res;

    if (n == 0) {
        return res;
    }

    res += sep;

    for (auto i = 0; i < n - 1; i++) {
        res += ", " + sep;
    }

    return res;
}

static std::string questionMarks(size_t n) { return makeSQLBinder(n, "?"); }

static std::string questionMarkPairs(size_t n) {
    return makeSQLBinder(n, "(?, ?)");
}

static std::string unigramPair(size_t n) { return makeSQLBinder(n, "(?, 1)"); }

static std::string bigramTriple(size_t n) {
    return makeSQLBinder(n, "(?, ?, 1)");
}

const std::string SELECT_DictionaryInputs = "SELECT id, input, output FROM dictionary";

const std::string SELECT_TrieWords = "SELECT DISTINCT ascii FROM trie_map";

const std::string INSERT_TrieMap(size_t n) {
    boost::format sql("INSERT INTO trie_map(ascii, dictionary_id) VALUES %1%");
    sql % questionMarkPairs(n);
    return sql.str();
}

const std::string CREATE_TrieMapTable = R"(
DROP TABLE IF EXISTS trie_map;
CREATE TABLE trie_map (
    id INTEGER PRIMARY KEY,
    ascii TEXT,
    dictionary_id INTEGER,
    FOREIGN KEY (dictionary_id) REFERENCES dictionary (id),
    UNIQUE(ascii, dictionary_id)
))";

const std::string INDEX_TrieMapTable =
    "CREATE INDEX trie_map_ascii_idx ON trie_map(ascii)";

const std::string SELECT_Unigram = "SELECT * FROM unigram_freq WHERE gram = ?";

const std::string SELECT_Syllables =
    "SELECT DISTINCT syl FROM syllables_by_freq ORDER BY id ASC";

const std::string SELECT_Dictionary_1 =
    R"(SELECT dictionary.*
FROM trie_map
INNER JOIN dictionary
ON trie_map.dictionary_id = dictionary.id
WHERE trie_map.ascii = ?
ORDER BY
    dictionary.weight DESC,
    dictionary.chhan_id ASC)";

const std::string SELECT_Dictionary_N(size_t n) {
    boost::format sql(
        R"(
SELECT
    dictionary.*
FROM trie_map
INNER JOIN dictionary
ON trie_map.dictionary_id = dictionary.id
WHERE trie_map.ascii IN
(%1%)
ORDER BY
dictionary.weight DESC,
dictionary.chhan_id ASC)");

    sql % questionMarks(n);
    return sql.str();
}

const std::string SELECT_DictionaryWithUnigrams(size_t n) {
    boost::format sql(R"(
WITH d AS (
    SELECT
        dictionary.*,
        trie_map.ascii
    FROM trie_map
    INNER JOIN dictionary
    ON trie_map.dictionary_id = dictionary.id
    WHERE trie_map.ascii IN (%1%)
)
SELECT
    d.*,
    CASE WHEN unigram_freq.n IS NULL THEN 0 ELSE unigram_freq.n END AS unigram_n
FROM d
LEFT JOIN unigram_freq
ON d.output = unigram_freq.gram
ORDER BY
    unigram_freq.n DESC,
    d.input_length DESC,
    d.weight DESC,
    d.chhan_id ASC
)");

    sql % questionMarks(n);
    return sql.str();
}

const std::string SELECT_Bigrams(size_t n) {
    boost::format sql(R"(
SELECT rgram, n
FROM bigram_freq
WHERE lgram = ? AND rgram in (%1%))");

    sql % questionMarks(n);
    return sql.str();
}

const std::string UPSERT_OneGram = R"(
INSERT INTO unigram_freq (gram, n) VALUES (?, 1)
ON CONFLICT DO UPDATE SET n = n + 1 WHERE gram = ?
)";

const std::string UPSERT_Unigrams(size_t n) {
    boost::format sql(R"(
INSERT INTO unigram_freq (gram, n) VALUES %1%
ON CONFLICT DO UPDATE SET n = n + 1 WHERE gram IN (%2%))");

    sql % unigramPair(n) % questionMarks(n);
    return sql.str();
}

const std::string UPSERT_Bigrams(size_t n) {
    boost::format sql(R"(
INSERT INTO bigram_freq (lgram, rgram, n) VALUES %1%
ON CONFLICT DO UPDATE SET n = n + 1 WHERE (lgram, rgram) IN ( 
VALUES %2% ))");

    sql % bigramTriple(n) % questionMarkPairs(n);

    return sql.str();
}

} // namespace SQL
} // namespace TaiKey