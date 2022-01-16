#include "pch.h"

#include "TextEngine.h"

namespace Khiin {

 HRESULT TextEngine::onTestKey(WPARAM wParam, BOOL *pConsumable) {
    if (wParam == 0x41) {
        *pConsumable = true;
    } else {
        *pConsumable = false;
    }
    return S_OK;
}

 HRESULT TextEngine::onKey(WPARAM wParam, std::string *output) {
    if (wParam == 0x41) {
        buffer += 'r';
    }

    *output = buffer;
    return S_OK;
}

 HRESULT TextEngine::clear() {
    buffer.clear();
    return S_OK;
}


} // namespace Khiin
