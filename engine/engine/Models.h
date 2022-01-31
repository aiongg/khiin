#pragma once

#include <string>

namespace khiin::engine {

struct DictionaryRow {
    int id;
    int chhan_id;
    std::string input;
    std::string output;
    int weight;
    int color;
    std::string hint;
};

} // namespace khiin::engine
