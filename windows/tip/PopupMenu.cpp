#include "pch.h"

#include "PopupMenu.h"

#include <variant>

#include "RenderFactory.h"
#include "TextService.h"
#include "UiStrings.h"

namespace khiin::win32 {
namespace {
using namespace messages;
using namespace winrt;
using namespace strings;
using namespace D2D1;

const std::wstring kMenuWindowClassName = L"PopupMenuWindow";

struct Separator {};

struct TextItem {
    bool checked = 0;
    uint32_t icon_rid = 0;
    uint32_t text_rid = 0;
    com_ptr<IDWriteTextLayout> layout = nullptr;
};

using MenuItem = std::variant<Separator, TextItem>;

static const DWORD kDwStyle = WS_BORDER | WS_POPUP;
static const DWORD kDwExStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;

std::vector<MenuItem> GetMenuItems() {
    auto ret = std::vector<MenuItem>();
    ret.push_back(MenuItem(TextItem{true, IDI_MAINICON, IDS_CONTINUOUS_MODE}));
    ret.push_back(MenuItem(TextItem{false, 0, IDS_BASIC_MODE}));
    ret.push_back(MenuItem(Separator{}));
    ret.push_back(MenuItem(TextItem{false, 0, IDS_MANUAL_MODE}));
    ret.push_back(MenuItem(TextItem{false, 0, IDS_DIRECT_MODE}));
    return ret;
}

constexpr int kBulletColWidth = 24;
constexpr int kIconColWidth = 32;
constexpr int kPadding = 8;
constexpr int kSepWidth = 1;
constexpr int kFontSize = 18;
constexpr int kRowHeight = 32;
const std::string kFontName = "Microsoft JhengHei UI Regular";

int TaskbarHeight() {
    RECT rcWork{};
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);

    RECT rcFull{};
    rcFull.right = ::GetSystemMetrics(SM_CXFULLSCREEN);
    rcFull.bottom = ::GetSystemMetrics(SM_CYFULLSCREEN);

    // TODO: 40 is the default for most users
    return 40;
}

float CenterTextLayoutY(IDWriteTextLayout *layout, float available_height) {
    DWRITE_TEXT_METRICS dwt_metrics;
    check_hresult(layout->GetMetrics(&dwt_metrics));
    return (available_height - dwt_metrics.height) / 2;
}

class PopupMenuImpl : public PopupMenu {
  public:
    PopupMenuImpl(TextService *pservice) {
        m_service.copy_from(pservice);
        BaseWindow::Create(NULL, kDwStyle, kDwExStyle);
        m_items = GetMenuItems();
    }

    virtual void Show(POINT pt) override {
        m_origin = pt;
        EnsureGraphicsResources();
        CalculateLayout();
        GuiWindow::Show();
    }

    virtual std::wstring const &ClassName() const override {
        return kMenuWindowClassName;
    }

  private:
    virtual void OnConfigChanged(AppConfig *config) override {
        GuiWindow::OnConfigChanged(config);
        ::RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    }

    virtual bool OnClick(Point pt) override {
        if (!ClientHitTest(pt)) {
            Hide();
            return false;
        }

        // ClientDp(pt);
        // auto item = m_layout_grid.GetItemByHit(pt);
        // if (item) {
        //    NotifyCandidateSelectListeners(item->candidate);
        //}
        return true;
    }

    void DiscardGraphicsResources() {
        m_target = nullptr;
        m_brush = nullptr;
        m_textformat = nullptr;
    }

    void EnsureGraphicsResources() {
        EnsureRenderTarget();
        if (!m_textformat) {
            m_textformat = m_factory->CreateTextFormat(kFontName, 16.0f);
        }
        if (!m_brush) {
            m_target->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0, 0), m_brush.put());
        }
    }

    void CalculateLayout() {
        auto width = 0;
        auto row_height = 0;
        auto n_text_items = 0;
        auto n_separators = 0;

        for (auto &itemv : m_items) {
            if (std::holds_alternative<Separator>(itemv)) {
                ++n_separators;
                continue;
            }
            auto &item = std::get<TextItem>(itemv);
            item.layout =
                m_factory->CreateTextLayout(T(item.text_rid, m_language), m_textformat, m_max_width, m_max_height);
            DWRITE_TEXT_METRICS metrics{};
            check_hresult(item.layout->GetMetrics(&metrics));
            width = std::max(width, static_cast<int>(metrics.width));
            row_height = std::max(row_height, static_cast<int>(metrics.height));
            ++n_text_items;
        }

        row_height = std::max(row_height, kRowHeight);
        width += kPadding * 2 + kBulletColWidth + kIconColWidth;

        auto x = m_origin.x - width;
        auto y = m_origin.y - row_height - 40;

        auto height = row_height * n_text_items + n_separators * kPadding + kPadding * 2;
        m_row_height = static_cast<float>(row_height);

        if (x < 0) {
            x = 40;
        }

        if (y < 0) {
            y = 40;
        }

        ::SetWindowPos(m_hwnd, 0, m_origin.x - width / 2, m_origin.y - height - 40, width, height,
                       SWP_NOACTIVATE | SWP_NOZORDER);
        ::RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    }

    void DrawSeparator(float top, float stroke_width = 1.0f) {
        RECT rc{};
        ::GetClientRect(m_hwnd, &rc);
        auto width = static_cast<float>(rc.right) - static_cast<float>(rc.left);
        m_brush->SetColor(m_colors.bg_selected);
        m_target->DrawLine(Point2F(0, top), Point2F(width, top), m_brush.get(), stroke_width);
    }

    void Draw() {
        m_target->Clear(m_colors.bg);
        auto origin = Point2F(kPadding, kPadding);

        for (auto &itemv : m_items) {
            if (std::holds_alternative<Separator>(itemv)) {
                DrawSeparator(origin.y + kPadding / 2);
                origin.y += kPadding;
                continue;
            }
            auto &item = std::get<TextItem>(itemv);

            if (item.icon_rid) {
                auto hicon = ::LoadIcon(m_service->hmodule(), MAKEINTRESOURCE(item.icon_rid));
                auto bmp = m_factory->CreateBitmap(m_target, hicon);
                auto x = origin.x + kBulletColWidth + (kIconColWidth - 16) / 2;
                auto y = origin.y + (m_row_height - 16) / 2;
                m_target->DrawBitmap(bmp.get(), D2D1::RectF(x, y, x + 16, y + 16));
            }
            {
                auto o = origin;
                o.x += kBulletColWidth + kIconColWidth;
                o.y += CenterTextLayoutY(item.layout.get(), m_row_height);
                m_brush->SetColor(m_colors.text);
                m_target->DrawTextLayout(o, item.layout.get(), m_brush.get());
            }
            origin.y += m_row_height;
        }
    }

    virtual void Render() override {
        try {
            EnsureGraphicsResources();

            PAINTSTRUCT ps;
            RECT rc;
            ::GetClientRect(m_hwnd, &rc);
            ::BeginPaint(m_hwnd, &ps);

            m_target->BindDC(ps.hdc, &rc);
            m_target->BeginDraw();
            Draw();
            auto hr = m_target->EndDraw();
            if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET) {
                DiscardGraphicsResources();
            }
            ::EndPaint(m_hwnd, &ps);
        } catch (...) {
            DiscardGraphicsResources();
        }
    }

    com_ptr<TextService> m_service = nullptr;
    com_ptr<ID2D1SolidColorBrush> m_brush = nullptr;
    com_ptr<IDWriteTextFormat> m_textformat = nullptr;
    com_ptr<IDWriteTextLayout> m_layout = nullptr;
    POINT m_origin{};
    std::vector<MenuItem> m_items;
    float m_row_height = kRowHeight;
};
} // namespace

PopupMenu *PopupMenu::Create(TextService *pservice) {
    return new PopupMenuImpl(pservice);
}

} // namespace khiin::win32
