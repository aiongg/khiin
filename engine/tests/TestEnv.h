#pragma once

#include <gtest/gtest.h>

#include "Engine.h"

class TestEnv : public ::testing::Environment {
  public:
    static khiin::engine::Engine *engine();
    virtual ~TestEnv() = default;
    virtual void SetUp();
    virtual void TearDown();
};
