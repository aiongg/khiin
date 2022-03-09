#pragma once

namespace khiin {
namespace proto {
class AppConfig;
}

namespace engine {

class ConfigChangeListener {
  public:
    virtual void OnConfigChanged(proto::AppConfig *config) = 0;
};

} // namespace engine
} // namespace khiin