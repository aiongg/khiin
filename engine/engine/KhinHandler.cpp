#include "KhinHandler.h"

#include <string>

#include "BufferElement.h"
#include "SyllableParser.h"

namespace khiin::engine {
namespace {

bool is_all_hyphens(std::string_view str) {
    for (auto c : str) {
        if (c != '-') {
            return false;
        }
    }
    return true;
}

void AutokhinBufferImpl(SyllableParser *parser, bool autokhin_enabled, std::vector<BufferElement> &buffer) {
    bool autokhin = false;
    auto it = buffer.begin();

    while (it != buffer.end()) {
        auto raw = it->raw();
        if (is_all_hyphens(raw) && raw.size() >= 2) {
            autokhin = true;

            if (raw.size() == 2) {
                it = buffer.erase(it);
                if (it == buffer.end()) {
                    auto elem = TaiText::FromRawSyllable(parser, "--");
                    buffer.insert(it, BufferElement(elem));
                    break;
                }
            } else {
                raw.erase(raw.end() - 2, raw.end());
                it->Replace(std::string(raw));
                if (auto next = std::next(it); next == buffer.end()) {
                    auto elem = TaiText::FromRawSyllable(parser, "--");
                    buffer.insert(it, BufferElement(elem));
                    break;
                }
                it = std::next(it, 1);
            }

            if (!it->SetKhin(KhinKeyPosition::Start, '-')) {
                autokhin = false;
            }

            if (it != buffer.end()) {
                ++it;
            }

            continue;
        }

        if (autokhin_enabled && autokhin && !it->SetKhin(KhinKeyPosition::Virtual, 0)) {
            autokhin = false;
        }

        ++it;
    }
}

} // namespace

void KhinHandler::AutokhinBuffer(SyllableParser *parser, bool autokhin_enabled, std::vector<BufferElement> &buffer) {
    AutokhinBufferImpl(parser, autokhin_enabled, buffer);
}

} // namespace khiin::engine