#pragma once

#include "messages.h"

namespace khiin::engine {

class ConfigChangeListener {
    virtual void OnConfigChanged(messages::AppConfig config) = 0;
};

} // namespace khiin::engine