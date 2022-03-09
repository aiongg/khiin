#include "pch.h"

#include "PopupMenu.h"

#include <variant>

#include "Geometry.h"
#include "RenderFactory.h"
#include "TextService.h"
#include "UiStrings.h"

namespace khiin::win32 {
namespace {
using namespace proto;
using namespace winrt;
using namespace strings;
using namespace D2D1;
using namespace geometry;

const std::wstring kMenuWindowClassName = L"PopupMenuWindow";

struct MenuItem {
    bool separator = false;
    bool checked = false;
    uint32_t icon_rid = 0;
    uint32_t text_rid = 0;
    Rect rc;
    com_ptr<IDWriteTextLayout> layout = nullptr;
};

MenuItem TextItem(uint32_t text_rid, uint32_t icon_rid = 0, bool checked = false) {
    auto item = MenuItem();
    item.text_rid = text_rid;
    item.icon_rid = icon_rid;
    item.checked = checked;
    return item;
}

MenuItem Separator() {
    auto item = MenuItem();
    item.separator = true;
    return item;
}

uint32_t CheckDarkModeIcon(uint32_t icon_rid, ColorScheme const &colors) {
    auto avg = (colors.text.r + colors.text.g + colors.text.b) / 3;
    if (avg >= 0.5) {
        switch (icon_rid) {
        case IDI_MODE_CONTINUOUS:
            return IDI_MODE_CONTINUOUS_W;
        case IDI_MODE_BASIC:
            return IDI_MODE_BASIC_W;
        case IDI_MODE_PRO:
            return IDI_MODE_PRO_W;
        case IDI_MODE_ALPHA:
            return IDI_MODE_ALPHA_W;
        case IDI_SETTINGS:
            return IDI_SETTINGS_W;
        }
    }

    return icon_rid;
}

static const DWORD kDwStyle = WS_POPUP;
static const DWORD kDwExStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;

std::vector<MenuItem> GetMenuItems() {
    auto ret = std::vector<MenuItem>();
    ret.push_back(TextItem(IDS_CONTINUOUS_MODE, IDI_MODE_CONTINUOUS, true));
    ret.push_back(TextItem(IDS_BASIC_MODE, IDI_MODE_BASIC));
    ret.push_back(TextItem(IDS_MANUAL_MODE, IDI_MODE_PRO));
    ret.push_back(TextItem(IDS_DIRECT_MODE, IDI_MODE_ALPHA));
    ret.push_back(Separator());
    ret.push_back(TextItem(IDS_OPEN_SETTINGS, IDI_SETTINGS));
    return ret;
}

constexpr int kTaskbarHeight = 48;
constexpr int kBulletColWidth = 24;
constexpr int kIconColWidth = 32;
constexpr int kVPad = 8;
constexpr int kHPad = 16;
constexpr int kSepWidth = 1;
constexpr int kFontSize = 18;
constexpr int kRowHeight = 34;
constexpr int kIconSize = 16;
const std::string kFontName = "Microsoft JhengHei UI Regular";

struct MenuMetrics {
    float y_adjust = 50.0f;
    float bullet_col = 24.0f;
    float icon_col = 32.0f;
    float vpad = 8.0f;
    float hpad = 16.0f;
    float sep_thickness = 1.0f;
    float font_size = 18.0f;
    float row_height = 32.0f;
    float icon_size = 16.0f;
};

MenuMetrics ScaledMetrics(float scale) {
    auto m = MenuMetrics();
    m.y_adjust *= scale;
    m.bullet_col *= scale;
    m.icon_col *= scale;
    m.vpad *= scale;
    m.hpad *= scale;
    m.row_height *= scale;
}

int TaskbarHeight() {
    RECT rcWork{};
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);

    RECT rcFull{};
    rcFull.right = ::GetSystemMetrics(SM_CXFULLSCREEN);
    rcFull.bottom = ::GetSystemMetrics(SM_CYFULLSCREEN);

    // TODO: 40 is the default for most users
    return 40;
}

int CenterTextLayoutY(IDWriteTextLayout *layout, int available_height) {
    auto h = static_cast<float>(available_height);
    DWRITE_TEXT_METRICS dwt_metrics;
    check_hresult(layout->GetMetrics(&dwt_metrics));
    return static_cast<int>((h - dwt_metrics.height) / 2.0f);
}

class PopupMenuImpl : public PopupMenu {
  public:
    PopupMenuImpl(TextService *pservice) {
        m_service.copy_from(pservice);
        BaseWindow::Create(NULL, kDwStyle, kDwExStyle);
        m_items = GetMenuItems();
    }

    virtual void Show(POINT pt) override {
        if (!DpiAware()) {
            m_origin = Point{ToDp(pt.x), ToDp(pt.y)};
        } else {
            m_origin = Point{pt.x, pt.y};
        }
        EnsureGraphicsResources();
        CalculateLayout();
        GuiWindow::Show();
    }

    virtual std::wstring const &ClassName() const override {
        return kMenuWindowClassName;
    }

  private:
    void Redraw() {
        ::RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    }

    int ItemHitTest(Point pt) {
        auto ret = -1;

        if (!ClientHitTest(pt)) {
            return ret;
        }

        pt.x = ToDp(pt.x);
        pt.y = ToDp(pt.y);

        auto begin = m_items.cbegin();
        auto end = m_items.cend();
        for (auto it = begin; it != end; ++it) {
            // KHIIN_DEBUG("Move point: {},{}", pt.x, pt.y);
            // KHIIN_DEBUG("Item rect: {}, {}, {}, {}", item.rc.left(), item.rc.top(), item.rc.right(),
            // item.rc.bottom());
            if (it->rc.Hit(pt)) {
                ret = std::distance(begin, it);
                break;
            }
        }

        return ret;
    }

    virtual void OnConfigChanged(AppConfig *config) override {
        GuiWindow::OnConfigChanged(config);
        Redraw();
    }

    virtual void OnMouseMove(Point pt) override {
        auto idx = ItemHitTest(pt);
        auto redraw = idx != m_highlighted_index;
        m_highlighted_index = idx;

        if (redraw) {
            Redraw();
        }
    }

    virtual bool OnClick(Point pt) override {
        auto idx = ItemHitTest(pt);
        Hide();

        if (idx < 0 || idx >= static_cast<int>(m_items.size())) {
            return false;
        }

        auto &item = m_items.at(idx);

        if (item.checked) {
            return true;
        }

        switch (item.text_rid) {
        case IDS_CONTINUOUS_MODE:
            m_service->OnInputModeSelected(InputMode::IM_CONTINUOUS);
            break;
        case IDS_BASIC_MODE:
            m_service->OnInputModeSelected(InputMode::IM_BASIC);
            break;
        case IDS_MANUAL_MODE:
            m_service->OnInputModeSelected(InputMode::IM_PRO);
            break;
        case IDS_DIRECT_MODE:
            m_service->OnInputModeSelected(InputMode::IM_ALPHA);
            break;
        case IDS_OPEN_SETTINGS:
            m_service->OpenSettingsApplication();
            break;
        }

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

    int WorkAreaBottom() {
        auto rc = RECT{};
        ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
        return rc.bottom;
    }

    void CalculateLayout() {
        auto max_item_width = 0;
        auto max_row_height = 0;

        for (auto &item : m_items) {
            if (item.separator) {
                continue;
            }

            item.layout =
                m_factory->CreateTextLayout(T(item.text_rid, m_language), m_textformat, m_max_width, m_max_height);
            DWRITE_TEXT_METRICS metrics{};
            check_hresult(item.layout->GetMetrics(&metrics));
            max_item_width = std::max(max_item_width, static_cast<int>(metrics.width));
            max_row_height = std::max(max_row_height, static_cast<int>(metrics.height));
        }

        auto width = kBulletColWidth + kIconColWidth + max_item_width;
        auto row_height = std::max(kRowHeight, max_row_height + kVPad);

        auto total_width = width + kHPad * 2;
        auto total_height = kVPad;

        for (auto it = m_items.begin(); it != m_items.end(); ++it) {
            auto &item = *it;

            if (item.separator) {
                item.rc = Rect{Point{0, total_height}, total_width, kVPad};
                total_height += kVPad;
                continue;
            } else {
                item.rc = Rect{Point{0, total_height}, total_width, row_height};
                total_height += row_height;
            }
        }

        total_height += kVPad;

        int w = total_width;
        int h = total_height;
        int x = m_origin.x;
        int y = WorkAreaBottom() - kHPad;

        if (DpiAware()) {
            w = ToPx(w);
            h = ToPx(h);
            y = WorkAreaBottom() - ToPx(kHPad);
        }

        x = x - w / 2;
        y = y - h;

        auto max_width = static_cast<int>(m_max_width);
        if (x + w > max_width) {
            x -= (x + w - max_width);
        }

        KHIIN_TRACE("PopupMenu: dpi={}, x={}, y={}, w={}, h={}", m_dpi, x, y, w, h);
        ::SetWindowPos(m_hwnd, 0, x, y, w, h, SWP_NOACTIVATE | SWP_NOZORDER);
        Redraw();
    }

    void DrawSeparator(MenuItem const &item, float stroke_width) {
        auto top = item.rc.topf() + item.rc.heightf() / 2;
        auto width = item.rc.widthf();
        m_target->DrawLine(Point2F(0, top), Point2F(width, top), m_brush.get(), stroke_width);
    }

    void DrawBullet(Rect const &rc) {
        auto center = rc.center();
        auto centerf = Point2F(static_cast<float>(center.x), static_cast<float>(center.y));
        m_target->FillEllipse(Ellipse(centerf, 2.0f, 2.0f), m_brush.get());
    }

    void DrawIcon(Rect const &rc, uint32_t icon_rid) {
        auto size = kIconSize;
        auto rid = CheckDarkModeIcon(icon_rid, m_colors);
        auto hicon = ::LoadIcon(m_service->hmodule(), MAKEINTRESOURCE(rid));
        auto bmp = m_factory->CreateBitmap(m_target, hicon);
        auto x = rc.left() + (rc.widthf() - size) / 2;
        auto y = rc.topf() + (rc.heightf() - size) / 2;
        m_target->DrawBitmap(bmp.get(), D2D1::RectF(x, y, x + size, y + size));
    }

    void DrawTextItem(MenuItem const &item) {
        auto origin = item.rc.origin();
        origin.x += kHPad;
        auto h = item.rc.height();

        if (item.checked) {
            auto rc = Rect{origin, kBulletColWidth, h};
            DrawBullet(rc);
        }

        if (item.icon_rid) {
            auto o = origin;
            o.x += kBulletColWidth;
            auto rc = Rect{o, kIconColWidth, h};
            DrawIcon(rc, item.icon_rid);
        }

        if (item.layout) {
            auto o = Point2F(static_cast<float>(origin.x), static_cast<float>(origin.y));
            o.x += kBulletColWidth + kIconColWidth;
            o.y += CenterTextLayoutY(item.layout.get(), h);
            m_target->DrawTextLayout(o, item.layout.get(), m_brush.get());
        }
    }

    void DrawHighlight(Rect const &rc) {
        m_target->FillRectangle(RectF(rc.leftf(), rc.topf(), rc.rightf(), rc.bottomf()), m_brush.get());
    }

    void Draw() {
        m_target->Clear(m_colors.bg);

        auto i = 0;
        auto begin = m_items.cbegin();
        auto end = m_items.cend();
        for (auto it = begin; it != end; ++it) {
            auto i = std::distance(begin, it);
            auto &item = *it;

            if (!item.separator) {
                if (m_highlighted_index == i) {
                    m_brush->SetColor(m_colors.bg_selected);
                    DrawHighlight(item.rc);
                }

                m_brush->SetColor(m_colors.text);
                DrawTextItem(item);
            } else {
                m_brush->SetColor(m_colors.accent);
                DrawSeparator(item, 1.0f);
            }
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
    com_ptr<IDWriteTextLayout> m_bullet = nullptr;
    Point m_origin{};
    std::vector<MenuItem> m_items;
    int m_highlighted_index = -1;
};
} // namespace

PopupMenu *PopupMenu::Create(TextService *pservice) {
    return new PopupMenuImpl(pservice);
}

} // namespace khiin::win32
