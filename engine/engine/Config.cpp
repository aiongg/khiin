#include "Config.h"

#include "proto/proto.h"

namespace khiin::engine {
namespace {

class ConfigImpl : public Config {
  public:
    ConfigImpl() {
        m_protoconf = proto::AppConfig::default_instance().New();
    }

    void UpdateAppConfig(const proto::AppConfig &proto_config) override {
        m_protoconf->CopyFrom(proto_config);
    }

    bool ime_enabled() override {
        if (m_protoconf->has_ime_enabled()) {
            return m_protoconf->ime_enabled().value();
        }

        return default_ime_enabled;
    }

    InputMode input_mode() override {
        switch (m_protoconf->input_mode()) {
        case proto::IM_PRO:
            return InputMode::Manual;
        case proto::IM_BASIC:
            return InputMode::Basic;
        case proto::IM_CONTINUOUS:
            return InputMode::Continuous;
        default:
            return default_input_mode;
        }
    }

    bool dotted_khin() override {
        if (m_protoconf->has_dotted_khin()) {
            return m_protoconf->dotted_khin().value();
        }

        return default_dotted_khin;
    }

    bool autokhin() override {
        if (m_protoconf->has_autokhin()) {
            return m_protoconf->autokhin().value();
        }

        return default_autokhin;
    }

    bool telex() override {
        if (m_protoconf->has_telex_enabled()) {
            return m_protoconf->telex_enabled().value();
        }

        return default_telex_enabled;
    }

    char telex_t2() override {
        if (m_protoconf->has_key_config()) {
            auto &keyconf = m_protoconf->key_config();
            if (!keyconf.telex_t2().empty()) {
                return keyconf.telex_t2().front();
            }
        }

        return default_telex_t2;
    }

    char telex_t3() override {
        if (m_protoconf->has_key_config()) {
            auto &keyconf = m_protoconf->key_config();
            if (!keyconf.telex_t3().empty()) {
                return keyconf.telex_t3().front();
            }
        }

        return default_telex_t3;
    }

    char telex_t5() override {
        if (m_protoconf->has_key_config()) {
            auto &keyconf = m_protoconf->key_config();
            if (!keyconf.telex_t5().empty()) {
                return keyconf.telex_t5().front();
            }
        }

        return default_telex_t5;
    }

    char telex_t6() override {
        return 0;
    }

    char telex_t7() override {
        if (m_protoconf->has_key_config()) {
            auto &keyconf = m_protoconf->key_config();
            if (!keyconf.telex_t7().empty()) {
                return keyconf.telex_t7().front();
            }
        }

        return default_telex_t7;
    }

    char telex_t8() override {
        if (m_protoconf->has_key_config()) {
            auto &keyconf = m_protoconf->key_config();
            if (!keyconf.telex_t8().empty()) {
                return keyconf.telex_t8().front();
            }
        }

        return default_telex_t8;
    }

    char telex_t9() override {
        if (m_protoconf->has_key_config()) {
            auto &keyconf = m_protoconf->key_config();
            if (!keyconf.telex_t9().empty()) {
                return keyconf.telex_t9().front();
            }
        }

        return default_telex_t9;
    }

    char telex_khin() override {
        if (m_protoconf->has_key_config()) {
            auto &keyconf = m_protoconf->key_config();
            if (!keyconf.telex_khin().empty()) {
                return keyconf.telex_khin().front();
            }
        }

        return default_telex_khin;
    }

    char alt_hyphen() override {
        if (m_protoconf->has_key_config()) {
            auto &keyconf = m_protoconf->key_config();
            if (!keyconf.alt_hyphen().empty()) {
                return keyconf.alt_hyphen().front();
            }
        }

        return default_alt_hyphen;
    }

    std::string nasal() override {
        if (m_protoconf->has_key_config()) {
            auto &keyconf = m_protoconf->key_config();
            if (!keyconf.nasal().empty()) {
                return keyconf.nasal();
            }
        }

        return default_nasal;
    }

    std::string dot_above_right() override {
        if (m_protoconf->has_key_config()) {
            auto &keyconf = m_protoconf->key_config();
            if (!keyconf.dot_above_right().empty()) {
                return keyconf.dot_above_right();
            }
        }

        return default_dotaboveright;
    }

    char dots_below() override {
        if (m_protoconf->has_key_config()) {
            auto &keyconf = m_protoconf->key_config();
            if (!keyconf.dots_below().empty()) {
                return keyconf.dots_below().front();
            }
        }

        return default_dotsbelow;
    }

    bool uppercase_nasal() override {
        if (m_protoconf->has_key_config()) {
            auto &keyconf = m_protoconf->key_config();
            if (keyconf.has_enable_uppercase_nasal()) {
                return keyconf.enable_uppercase_nasal().value();
            }
        }

        return false;
    }

    void set_dotted_khin(bool value) override {
        m_protoconf->mutable_dotted_khin()->set_value(value);
    }

    void set_autokhin(bool value) override {
        m_protoconf->mutable_autokhin()->set_value(value);
    }

    proto::AppConfig *m_protoconf = nullptr;

    bool default_ime_enabled = true;
    InputMode default_input_mode = InputMode::Continuous;
    bool default_telex_enabled = false;
    bool default_dotted_khin = true;
    bool default_autokhin = true;
    std::string default_nasal = "nn";
    std::string default_dotaboveright = "ou";
    char default_dotsbelow = 'r';
    char default_telex_t2 = 's';
    char default_telex_t3 = 'f';
    char default_telex_t5 = 'l';
    // char default_telex_t6 = 'z';
    char default_telex_t7 = 'j';
    char default_telex_t8 = 'j';
    char default_telex_t9 = 'w';
    char default_telex_khin = 'q';
    char default_alt_hyphen = 0;
};

} // namespace

std::unique_ptr<Config> Config::Default() {
    return std::make_unique<ConfigImpl>();
}

} // namespace khiin::engine
