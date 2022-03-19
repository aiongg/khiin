#pragma once

#ifdef _WIN32
#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#endif

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "engine/Database.h"

#include "TestEnv.h"

namespace {
static khiin::engine::Engine *g_engine = nullptr;
}

void TestEnv::SetUp() {
    g_engine = khiin::engine::Engine::Create("./khiin_test.db");
    g_engine->database()->ClearNGramsData();
}

void TestEnv::TearDown() {
    delete g_engine;
}

khiin::engine::Engine* TestEnv::engine() {
    return g_engine;
}

int main(int argc, char *argv[]) {
    ::testing::AddGlobalTestEnvironment(new TestEnv());
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
