#pragma once

#include <string>

#include <boost/property_tree/ptree.hpp>

namespace taikey {

enum class InputMode {
    Normal,
    Pro,
};

enum class ToneKeys {
    Numeric,
    Telex,
};

enum class ToneMode {
    Fuzzy,
    Exact,
};

enum class CommitMode {
    Lazy,
    Quick,
};

struct Settings {
    InputMode inputMode;
    ToneKeys toneKeys;
    ToneMode toneMode;
    CommitMode commitMode;
    bool englishCandidates;
    bool capitalizeEnglish;
    bool easyHyphen;
    bool easyKhin;
    bool easyCh;
};

class Config {
  public:
    Config();
    Config(std::string configFile);
    auto setConfigFile(std::string configFile) -> void;
    auto setPreference(std::string key, bool value) -> int;
    auto setPreference(std::string key, std::string value) -> int;
    auto getSettings() -> const Settings&;

  private:
    std::string configFile_;
    boost::property_tree::ptree json_;
    Settings settings_;
};

} // namespace taikey