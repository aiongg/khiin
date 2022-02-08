#pragma once

#include <string>

namespace khiin::engine {

struct TaiToken {
    int id;
    int chhan_id;
    std::string input;
    std::string output;
    int weight;
    int category;
    std::string annotation;
};

} // namespace khiin::engine
