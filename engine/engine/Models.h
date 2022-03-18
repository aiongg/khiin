#pragma once

#include <string>

namespace khiin::engine {

struct InputByFreq {
    int id = 0;
    std::string input;
};

struct TaiToken {
    int id = 0;
    int chhan_id = 0;
    int input_id = 0;
    std::string input;
    std::string output;
    int weight = 0;
    int category = 0;
    std::string annotation;
};

struct Gram {
    std::string value;
    int count = 0;
};

struct Punctuation {
    int id = 0;
    std::string input;
    std::string output;
    std::string annotation;
};

struct Emoji {
    int category = 0;
    std::string value;
};

} // namespace khiin::engine
