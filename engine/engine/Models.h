#pragma once

#include <string>

namespace khiin::engine {

struct InputByFreq {
    int id;
    std::string input;
};

struct TaiToken {
    int id;
    int chhan_id;
    int input_id;
    std::string input;
    std::string output;
    int weight;
    int category;
    std::string annotation;
};

struct Punctuation {
    int id;
    std::string input;
    std::string output;
    std::string annotation;
};

} // namespace khiin::engine
