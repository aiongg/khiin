
#include "engine.h"

namespace TaiKey {

//TKEngine::TKEngine() {}

TKEngine::TKEngine(std::string tkFolder)
    : tkFolder_(tkFolder), database_((tkFolder_ /= DB_FILE).string()) {
    config_.setConfigFile((tkFolder_ /= CONFIG_FILE).string());

    splitter_ = Splitter(database_.selectSyllableList());
    trie_ = Trie(database_.selectTrieWordlist(), database_.selectSyllableList());
    candidateFinder = std::make_unique<CandidateFinder>(
        CandidateFinder(database_, splitter_, trie_));
    buffer_ = std::make_unique<BufferManager>(BufferManager(*candidateFinder.get()));
}

} // namespace TaiKey