#pragma once

#include <memory>

namespace khiin {
namespace proto {
class AppConfig;
}

namespace engine {

class ConfigChangeListener {
  public:
    virtual void OnConfigChanged(proto::AppConfig *config) = 0;
};

std::unique_ptr<proto::AppConfig> DefaultAppConfig();

} // namespace engine
} // namespace khiin
