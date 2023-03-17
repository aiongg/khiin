#include "UserDictionaryParser.h"

#include <fstream>
#include <vector>

#include "utils/unicode.h"

namespace khiin::engine {
namespace {

constexpr char kNewLine = '\n';
constexpr char kCarriageReturn = '\r';
constexpr char kCommentMarker = '#';
const std::vector<char> kSeparators = {' ', '\t'};
bool is_separator(char ch) {
    return std::find(kSeparators.begin(), kSeparators.end(), ch) != kSeparators.end();
}

class UserDictionaryParserImpl : public UserDictionaryParser {
  public:
    ~UserDictionaryParserImpl() override = default;

    void Open(std::string filename) {
        file = std::ifstream(filename);
    }

  private:
    void Clear() {
        current_line.clear();
        current_result.first.clear();
        current_result.second.clear();
    }

    bool Advance() override {
        return !file.eof() && ParseLine();
    }

    std::pair<std::string, std::string> GetRow() override {
        return current_result;
    }

    void ReadLine() {
        auto &str = current_line;
        bool is_comment = false;
        std::istream::sentry sentry(file, true);
        std::streambuf *buf = file.rdbuf();

        for (;;) {
            int c = buf->sbumpc();
            switch (c) {
            case std::streambuf::traits_type::eof():
                if (str.empty()) {
                    file.setstate(std::ios::eofbit);
                }
                return;
            case kNewLine:
                if (is_comment && str.empty()) {
                    is_comment = false;
                    continue;
                }
                return;
            case kCarriageReturn:
                if (buf->sgetc() == '\n') {
                    buf->sbumpc();
                }
                if (is_comment && str.empty()) {
                    is_comment = false;
                    continue;
                }
                return;
            case kCommentMarker:
                is_comment = true;
                continue;
            default: {
                if (is_comment || (isspace(c) && str.empty())) {
                    continue;
                }
                str += static_cast<char>(c);
            }
            }
        }
    }

    bool ParseLine() {
        for (;;) {
            if (file.eof()) {
                break;
            }

            Clear();
            ReadLine();
            auto &str = current_line;
            auto &res = current_result;

            unicode::trim(str);
            if (!utf8::is_valid(str)) {
                continue;
            }

            auto it = std::find_first_of(str.begin(), str.end(), kSeparators.begin(), kSeparators.end());

            if (it == str.end()) {
                continue;
            }

            res.first = std::string(str.begin(), it);

            if (std::any_of(res.first.cbegin(), res.first.cend(), [](unsigned char ch) {
                    return std::isgraph(ch) == 0;
                })) {
                continue;
            }

            while (is_separator(*it)) {
                ++it;
            }

            res.second = std::string(it, str.end());

            return true;
        }

        return false;
    }

    std::ifstream file;
    std::string current_line;
    std::pair<std::string, std::string> current_result;
};

} // namespace

std::unique_ptr<UserDictionaryParser> UserDictionaryParser::LoadFile(std::string filename) {
    auto udp = std::make_unique<UserDictionaryParserImpl>();
    udp->Open(filename);
    return udp;
}

} // namespace khiin::engine
