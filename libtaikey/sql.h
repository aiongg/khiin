#pragma once

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

namespace TaiKey {
namespace SQL {

const std::string SELECT_TrieWords = R"(SELECT DISTINCT ascii FROM trie_map)";

const std::string SELECT_Syllables =
    R"(SELECT DISTINCT syl FROM syllables_by_freq ORDER BY id ASC)";

const std::string SELECT_Dictionary_1 =
    R"(SELECT dictionary.*
FROM trie_map
INNER JOIN dictionary
ON trie_map.dictionary_id = dictionary.id
WHERE trie_map.ascii = ?
ORDER BY
    dictionary.weight DESC,
    dictionary.chhan_id ASC)";

const std::string SELECT_Dictionary_N(int n) {
    boost::format sql(
        R"(SELECT dictionary.*
    FROM trie_map
    INNER JOIN dictionary
    ON trie_map.dictionary_id = dictionary.id
    WHERE trie_map.ascii IN
    ( %1% )
    ORDER BY
    dictionary.weight DESC,
    dictionary.chhan_id ASC)");

    sql % boost::algorithm::join(std::vector<std::string>(n, "?"), ", ");

    return sql.str();
}

} // namespace SQL
} // namespace TaiKey