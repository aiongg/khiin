#pragma once

#include <filesystem>
#include <string>
#include <memory>

#include "buffer.h"
#include "candidates.h"
#include "config.h"
#include "db.h"
#include "syl_splitter.h"
#include "trie.h"

namespace TaiKey {

const std::string CONFIG_FILE = "taikey.json";
const std::string DB_FILE = "taikey.db";

class TKEngine {
  public:
    TKEngine(std::string tkFolder);

  private:
    std::filesystem::path tkFolder_;
    Buffer buffer_;
    TKDB database_;
    Config config_;
    Splitter splitter_;
    Trie trie_;
    std::unique_ptr<CandidateFinder> candidateFinder_;
};

} // namespace TaiKey