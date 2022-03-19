#pragma once

#include <gtest/gtest.h>

#include "Engine.h"

class TestEnv : public ::testing::Environment {
  public:
    //virtual ~TestEnv() = default;
    void SetUp() override;
    void TearDown() override;
    khiin::engine::Engine *engine();
};
