// libtaikey.cpp : Defines the entry point for the application.
//
#ifdef _WIN32
#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#endif

#include <unordered_map>
#include <boost/assign.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <unicode/uclean.h>
#include <utf8cpp/utf8.h>

#include "libtaikey.h"

using namespace std;
using namespace boost;
using namespace boost::locale;

namespace TaiKey
{

string defaultSettings = R"settings({
    "T2": "s"
})settings";

struct taikey_settings
{
    char key_T2;
    void load(const string &filename);
};

void taikey_settings::load(const string &filename = "")
{
    pt::ptree tree;
    if (filename == "")
    {
        stringstream ss;
        ss << defaultSettings;
        pt::read_json(ss, tree);
    }
    else
    {

    }

    key_T2 = tree.get<char>("T2");
}


enum class Tone
{
    NaT,
    T2,
    T3,
    T5,
    T6,
    T7,
    T8,
    T9,
    TK,
};

enum class Special
{
    NasalCombo,
    NasalSolo,
    OUCombo,
    OUSolo,
};

unordered_map<char, Tone> TONE_KEY_MAP = {
    {'s', Tone::T2},
    {'f', Tone::T3},
    {'l', Tone::T5},
    {'j', Tone::T7},
    {'y', Tone::T8},
    {'w', Tone::T9},
    {'q', Tone::TK}
};

unordered_map<Tone, string> TONE_UTF_MAP = {
        {Tone::T2, u8"\u0301"},
        {Tone::T3, u8"\u0300"},
        {Tone::T5, u8"\u0302"},
        {Tone::T6, u8"\u030c"},
        {Tone::T7, u8"\u0304"},
        {Tone::T8, u8"\u030d"},
        {Tone::T9, u8"\u0306"},
        {Tone::TK, u8"\u00b7"}
};

unordered_map<Special, char> SPEC_KEY_MAP = {
    { Special::NasalCombo, 'n' },
    { Special::OUCombo, 'u' }
};

string TONES[] = {
    u8"\u0301", u8"\u0300", u8"\u0302", u8"\u030c",
    u8"\u0304", u8"\u030d", u8"\u0306", u8"\u00b7"
};

string _getToneOfKey(const char c)
{
    auto t = TONE_KEY_MAP.find(c);
    if (t == TONE_KEY_MAP.end())
    {
        return "";
    }

    auto s = TONE_UTF_MAP.find(t->second);
    if (s == TONE_UTF_MAP.end())
    {
        return "";
    }

    return s->second;
}

string stripDiacritics(const string& s)
{
    string _s = normalize(s, norm_nfd);
    for (string tone : TONES)
    {
        size_t found = _s.find(tone);
        if (found != string::npos)
        {
            _s.erase(found, tone.size());
            break;
        }
    }
    return _s;
}

// s must be one syllable
string placeToneOnSyllable(const string& syllable, const string& tone)
{
    static regex e(u8"o[ae][mnptkh]");
    static string ordered_vowel_matches[] = { u8"o", u8"a", u8"e", u8"u", u8"i", u8"ng", u8"m" };
 
    string _s = stripDiacritics(syllable);

    BOOST_LOG_TRIVIAL(debug) << boost::format("stripped syl: %1%") % _s;

    smatch match;
    size_t found;

    if (regex_search(_s, match, e))
    {
        found = match.position() + 1;
    }
    else
    {
        for (string v : ordered_vowel_matches)
        {
            found = _s.find(v);

            if (found != string::npos)
            {
                break;
            }
        }
    }

    if (found == string::npos)
    {
        return syllable;
    }

    _s.insert(found + 1, tone);

    return _s;

}

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

EngineState TaiKey::TaiKeyEngine::pushChar(const char c)
{
    BOOST_LOG_TRIVIAL(debug) << boost::format("pushChar(%1%)") % c;

    if (_keyBuffer.empty())
    {
        _keyBuffer.push_back(c);
        return _state;
    }

    if (TONE_KEY_MAP.find(c) != TONE_KEY_MAP.end())
    {
        string tone = _getToneOfKey(c);
        _keyBuffer = normalize(placeToneOnSyllable(_keyBuffer, tone), norm_nfc);
    }
    else if (_keyBuffer.back() == 'o' && c == SPEC_KEY_MAP.at(Special::OUCombo))
    {
        _keyBuffer += u8"\u0358";
    }
    else if (_keyBuffer.back() == 'n' && c == SPEC_KEY_MAP.at(Special::NasalCombo))
    {
        replace_last(_keyBuffer, u8"n", u8"\u207f");
    }
    else if (c == '\b')
    {
        pop_back();
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

std::string TaiKeyEngine::getBuffer() const
{
    return normalize(_keyBuffer, norm_nfc);
}

void TaiKeyEngine::pop_back()
{
    if (_keyBuffer.empty())
    {
        return;
    }

    _keyBuffer.pop_back();
    while (!utf8::is_valid(_keyBuffer.begin(), _keyBuffer.end()))
    {
        _keyBuffer.pop_back();
    }
}

} // namespace TaiKey
