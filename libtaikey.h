#pragma once

#include <boost/locale.hpp>

namespace TaiKey
{

static bool READY = false;
inline void initialize()
{
    boost::locale::generator gen;
    std::locale loc = gen("");
    std::locale::global(loc);
    std::wcout.imbue(loc);
    READY = true;
}

enum class EngineState
{
    Valid,
};


class TaiKeyEngine
{

public:
    TaiKeyEngine();
    void reset();

    EngineState pushChar(char c);
    EngineState getState() const;

    std::string getBuffer();

private:
    std::string _keyBuffer;
    EngineState _state;
};


} // namespace TaiKey
