#pragma once

#include <vector>

namespace khiin::engine {

class SyllableParser;
class BufferElement;

class KhinHandler {
  public:
    static void AutokhinBuffer(SyllableParser* parser, bool autokhin_enabled, std::vector<BufferElement> &buffer);
};

}