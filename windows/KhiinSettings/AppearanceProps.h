#pragma once

#include "PropSheet.h"

namespace khiin::win32::settings {

class AppearanceProps : public PropSheet {
  public:
    AppearanceProps(messages::AppConfig *config, HINSTANCE instance, int template_id);
    virtual void Initialize() override;
    virtual void Finalize() override;

    void SetFinalizeCallback(std::function<void()> callback);

  private:
    messages::AppConfig *config;
    std::function<void()> m_finalize_cb;
};

} // namespace khiin::win32::settings
