// libtaikey.cpp : Defines the entry point for the application.
//

#include <boost/locale.hpp>
#include "libtaikey.h"

using namespace boost::locale;
using namespace std;

namespace TaiKey
{

// const wstring LEGAL_CHARACTERS = L"ABCEGHIJKLMNOPSTUabceghijklmnopstu";

TaiKeyEngine::TaiKeyEngine()
{
    if (!READY)
    {
        initialize();
    }

    _keyBuffer.reserve(10);
    reset();
}

void TaiKeyEngine::reset()
{
    _keyBuffer.clear();
    _state = EngineState::Valid;
}

EngineState TaiKey::TaiKeyEngine::pushChar(char c)
{

    if (c == u8's')
    {
        _keyBuffer.append(u8"\u0301");
    }
    else if (c == u8'f')
    {
        _keyBuffer.append(u8"\u0300");
    }
    else
    {
        _keyBuffer.push_back(c);
    }

    return _state;
}

EngineState TaiKeyEngine::getState() const
{
    return _state;
}

std::string TaiKeyEngine::getBuffer()
{
    return normalize(_keyBuffer, norm_nfc);
}

} // namespace TaiKey
