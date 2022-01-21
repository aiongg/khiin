#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "common.h"

namespace taikey {
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

const std::string SELECT_DictionaryInputs =
    "SELECT id, input, output FROM dictionary";

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

const std::string SELECT_Tokens(size_t n) {
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

const std::string CREATE_DummyDatabase() {
    return std::string(R"(BEGIN TRANSACTION;
    DROP TABLE IF EXISTS "version";
    CREATE TABLE IF NOT EXISTS "version" (
	    "key"	TEXT,
	    "value"	INTEGER
    );
    DROP TABLE IF EXISTS "syllables_by_freq";
    CREATE TABLE IF NOT EXISTS "syllables_by_freq" (
	    "id"	INTEGER,
	    "syl"	TEXT UNIQUE,
	    PRIMARY KEY("id")
    );
    DROP TABLE IF EXISTS "unigram_freq";
    CREATE TABLE IF NOT EXISTS "unigram_freq" (
	    "id"	INTEGER,
	    "gram"	TEXT NOT NULL UNIQUE,
	    "n"	INTEGER NOT NULL,
	    PRIMARY KEY("id")
    );
    DROP TABLE IF EXISTS "bigram_freq";
    CREATE TABLE IF NOT EXISTS "bigram_freq" (
	    "id"	INTEGER,
	    "lgram"	TEXT,
	    "rgram"	TEXT,
	    "n"	INTEGER NOT NULL,
	    PRIMARY KEY("id"),
	    UNIQUE("lgram","rgram")
    );
    DROP TABLE IF EXISTS "dictionary";
    CREATE TABLE IF NOT EXISTS "dictionary" (
	    "id"	INTEGER,
	    "chhan_id"	INTEGER,
	    "input"	TEXT NOT NULL,
	    "output"	TEXT NOT NULL,
	    "weight"	INTEGER,
	    "input_length"	INTEGER,
	    "color"	INTEGER,
	    "hint"	TEXT,
	    PRIMARY KEY("id"),
	    UNIQUE("input","output")
    );
    INSERT INTO "version" VALUES ('id',1),
     ('created_at',1639838788),
     ('last_updated',1639839434);
    INSERT INTO "syllables_by_freq" VALUES
     (1,'e'), (2,'a'), (3,'si'), (4,'bo'), (5,'lang'),
     (6,'chit'), (7,'chiah'), (8,'m'), (9,'i'), (10,'khi'),
     (11,'kong'), (12,'u'), (13,'lai'), (14,'ti'), (15,'be'),
     (16,'ke'), (17,'kau'), (18,'tioh'),  (19,'ka'), (20,'kiann'),
     (21,'li'), (22,'thau'), (23,'cho'), (24,'ki'), (25,'hou'),
     (26,'chai'), (27,'ho'), (28,'te'), (29,'goa'), (30,'to'),
     (31,'toa'), (32,'cheng'), (33,'su'), (34,'na'), (35,'sai'),
     (36,'chiu'), (37,'tai'), (38,'hoe'), (39,'che'), (40,'koe'),
     (41,'beh'), (42,'kha'), (43,'khoann'), (44,'seng'), (45,'boe'),
     (46,'sin'), (47,'chin'), (48,'chu'), (49,'kou'), (50,'chhiu'),
     (51,'au'), (52,'chinn'), (53,'lou'), (54,'chhui'), (55,'se'),
     (56,'chi'), (57,'bou'), (58,'ni'), (59,'ma'), (60,'lau'),
     (61,'sann'), (62,'teng'), (63,'kui'), (64,'bin'), (65,'tng'),
     (66,'ku'), (67,'tang'), (68,'hong'), (69,'eng'), (70,'ia'),
     (71,'pou'), (72,'kang'), (73,'sim'), (74,'keng'), (75,'chui'),
     (76,'sio'), (77,'jit'), (78,'oan'), (79,'siong'), (80,'tit'),
     (81,'hi'), (82,'ai'), (83,'koh'), (84,'khah'), (85,'in'),
     (86,'ui'), (87,'chhau'), (88,'leng'), (89,'koan'), (90,'tau'),
     (91,'tou'), (92,'thang'), (93,'chun'), (94,'long'), (95,'chhut'),
     (96,'put'), (97,'koann'), (98,'hu'), (99,'bi'), (100,'peh');
    INSERT INTO "dictionary" VALUES
     (1,85,'a','阿',1000,1,NULL,NULL),
     (2,631,'a','唖',1000,1,NULL,NULL);
    DROP INDEX IF EXISTS "unigram_freq_gram_idx";
    CREATE INDEX IF NOT EXISTS "unigram_freq_gram_idx" ON "unigram_freq" (
	    "gram"
    );
    DROP INDEX IF EXISTS "bigram_freq_gram_index";
    CREATE INDEX IF NOT EXISTS "bigram_freq_gram_index" ON "bigram_freq" (
	    "lgram",
	    "rgram"
    );
    COMMIT;
    )");
}

} // namespace SQL
} // namespace taikey