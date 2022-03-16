#pragma once

#include <vector>

namespace khiin::engine {

class Engine;
class Buffer;

class KhinHandler {
  public:
    static void AutokhinBuffer(Engine *engine, Buffer &buffer);
};

}