#pragma once

#include <vector>

namespace khiin::engine {

class SyllableParser;
class BufferElement;

class KhinHandler {
  public:
    static void AutokhinBuffer(SyllableParser* parser, std::vector<BufferElement> &buffer);
};

}