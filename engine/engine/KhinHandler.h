#pragma once

#include <vector>

namespace khiin::engine {

class Engine;
class BufferElement;

class KhinHandler {
  public:
    static void AutokhinBuffer(Engine *engine, std::vector<BufferElement> &buffer);
};

}