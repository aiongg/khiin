#pragma once

#include <memory>
#include <string>

namespace khiin {
namespace proto {
class AppConfig;
}

namespace engine {

class Config;
class KeyConfig;

class ConfigChangeListener {
  public:
    virtual void OnConfigChanged(Config *config) = 0;
};

enum class InputMode {
    Continuous,
    Basic,
    Manual,
};

enum class InputType {
    Numeric,
    Telex,
};

class Config {
  public:
    Config() = default;
    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;
    virtual ~Config() = 0;

    static std::unique_ptr<Config> Default();
    virtual void UpdateAppConfig(proto::AppConfig const &proto_config) = 0;

    virtual bool ime_enabled() = 0;
    virtual InputMode input_mode() = 0; 
    virtual bool dotted_khin() = 0;
    virtual bool autokhin() = 0;
    virtual bool telex() = 0;

    // Keys
    virtual char telex_t2() = 0;
    virtual char telex_t3() = 0;
    virtual char telex_t5() = 0;
    virtual char telex_t6() = 0;
    virtual char telex_t7() = 0;
    virtual char telex_t8() = 0;
    virtual char telex_t9() = 0;
    virtual char telex_khin() = 0;
    virtual char alt_hyphen() = 0;
    virtual std::string nasal() = 0;
    virtual std::string dot_above_right() = 0;
    virtual char dots_below() = 0;
    virtual bool uppercase_nasal() = 0;

    // Setters
    virtual void set_dotted_khin(bool value) = 0;
    virtual void set_autokhin(bool value) = 0;
};

} // namespace engine
} // namespace khiin
