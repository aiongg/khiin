#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "proto/proto.h"

#include "engine/BufferMgr.h"
#include "engine/Config.h"
#include "engine/Engine.h"

#include "TestEnv.h"

namespace khiin::engine {
using ::testing::Contains;

struct OrEqual {
    inline bool operator()(std::string test, std::string v1, std::string v2) {
        return test == v1 || test == v2;
    }

    inline bool operator()(std::string test, std::string v1, std::string v2, std::string v3) {
        return test == v1 || test == v2 || test == v3;
    }
};

class BufferMgr;
class Engine;
using Segments = google::protobuf::RepeatedPtrField<khiin::proto::Preedit_Segment>;

struct BufferMgrTestBase : TestEnv {
  protected:
    BufferMgr *bufmgr = nullptr;

    void input(std::string str);
    void curs_left(int n);
    void curs_right(int n);
    void curs_down(int n);
    void curs_up(int n);
    void key_bksp(int n);
    void key_del(int n);
    void spacebar(int n);
    void enter();

    khiin::proto::Preedit *get_preedit();
    Segments get_segments();
    khiin::proto::CandidateList *get_candidates();
    std::string display();
    size_t caret();
    std::vector<std::string> get_cand_strings();
    std::string CandidateAt(int candidate_index);
    int CandidateIndexOf(std::string candidate);

    // Expectations
    void ExpectDisplay(std::string value);
    void ExpectCaret(int caret_index);
    void ExpectBuffer(std::string value, int caret_index);
    void ExpectSegment(int segment_size, int segment_index, proto::SegmentStatus segment_status,
                       std::string segment_value, int caret_index);
    void ExpectSegment(int segment_size, int segment_index, proto::SegmentStatus segment_status);
    void ExpectEmpty();
    void ExpectCandidatesHidden();
    void ExpectCandidateSize(int candidate_size);
    void ExpectCandidate(std::string candidate);
    void ExpectCandidate(std::string candidate, int candidate_index, bool focused);
};

} // namespace khiin::engine
