#pragma once

#include <gtest/gtest.h>

#include "Engine.h"

namespace khiin::engine {

class TestEnv : public ::testing::Environment {
  public:
    void SetUp() override;
    Engine *engine();
};

}
