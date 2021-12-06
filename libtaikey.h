#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

#include <boost/locale.hpp>
#include <string>

namespace TaiKey {

static bool READY = false;
inline void initialize() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    boost::locale::generator gen;
    std::locale loc = gen("");
    std::locale::global(loc);
    std::wcout.imbue(loc);
    READY = true;
}

enum class EngineState {
    Valid,
};

class TaiKeyEngine {

  public:
    TaiKeyEngine();
    void reset();

    EngineState pushChar(char c);
    EngineState getState() const;
    std::string getBuffer() const;

  private:
    std::string _keyBuffer;
    EngineState _state;
    void pop_back();
};

} // namespace TaiKey
