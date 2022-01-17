#include "pch.h"

#include "TextEngine.h"

namespace Khiin {
HRESULT TextEngine::init() {
    return S_OK;
}

HRESULT TextEngine::uninit() {
    return S_OK;
}

HRESULT TextEngine::onTestKey(WPARAM wParam, BOOL *pConsumable) {
    if (wParam == 0x41) {
        *pConsumable = true;
    } else {
        *pConsumable = false;
    }
    return S_OK;
}

HRESULT TextEngine::onKey(WPARAM wParam, std::string *pOutput) {
    if (wParam == 0x41) {
        buffer += 'r';
    }

    *pOutput = buffer;
    return S_OK;
}

HRESULT TextEngine::clear() {
    buffer.clear();
    return S_OK;
}

HRESULT TextEngine::getCandidates(std::vector<std::string> *pCandidates) {
    if (candidates.empty()) {
        candidates.push_back(u8"枝");
        candidates.push_back(u8"乩");
        candidates.push_back(u8"機");
        candidates.push_back(u8"机");
        candidates.push_back(u8"箕");
        candidates.push_back(u8"飢");
        candidates.push_back(u8"幾");
        candidates.push_back(u8"支");
        candidates.push_back(u8"居");
        candidates.push_back(u8"基");
        candidates.push_back(u8"奇");
        candidates.push_back(u8"姬");
        candidates.push_back(u8"畿");
        candidates.push_back(u8"裾");
    }

    *pCandidates = candidates;
    return S_OK;
}

} // namespace Khiin
