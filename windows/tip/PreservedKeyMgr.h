#pragma once

namespace khiin::win32::tip {

struct TextService;

class PreservedKeyMgr {
  public:
    static PreservedKeyMgr *Create();
    virtual void Initialize(TextService *service) = 0;
    virtual void Shutdown() = 0;
};

}