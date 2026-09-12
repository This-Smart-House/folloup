#include "epaper_ui/menu_toggle.h"

#include <algorithm>
#include <string>
#include <vector>

#include "render_utils.h"

namespace epaper_ui {

UiRect MenuToggleBounds(int origin_x, int origin_y, const MenuToggleStyle& style)
{
    return {origin_x, origin_y, ClampPositive(style.width), ClampPositive(style.height)};
}

void DrawMenuToggle(uint8_t* framebuffer,
                    int raw_width,
                    int raw_height,
                    int portrait_width,
                    int portrait_height,
                    int origin_x,
                    int origin_y,
                    const MenuToggleState& state,
                    const MenuToggleStyle& style)
{
    const UiRect bounds = MenuToggleBounds(origin_x, origin_y, style);
    if (bounds.IsEmpty()) {
        return;
    }

    FillPortraitRect(framebuffer,
                     raw_width,
                     raw_height,
                     portrait_width,
                     portrait_height,
                     bounds,
                     style.background_color);

    const int border_height = ClampPositive(style.bottom_border_thickness);
    if (border_height > 0) {
        FillPortraitRect(framebuffer,
                         raw_width,
                         raw_height,
                         portrait_width,
                         portrait_height,
                         {bounds.x,
                          bounds.bottom() - std::min(border_height, bounds.height),
                          bounds.width,
                          std::min(border_height, bounds.height)},
                         style.border_color);
    }

    ToggleStyle toggle_style = style.toggle;
    const UiRect toggle_bounds = ToggleBounds(0, 0, toggle_style);
    const int toggle_x =
        bounds.right() - ClampPositive(style.horizontal_padding) - toggle_bounds.width;
    const int toggle_y = bounds.y + CenterOffset(bounds.height, toggle_bounds.height);
    DrawToggle(framebuffer,
               raw_width,
               raw_height,
               portrait_width,
               portrait_height,
               toggle_x,
               toggle_y,
               state.toggle_state,
               toggle_style);

    if (state.label_text.empty()) {
        return;
    }

    const int label_x = bounds.x + ClampPositive(style.horizontal_padding);
    const int line_height = LineHeight(style.role);

    if (style.max_label_lines <= 1) {
        DrawTypographyText(framebuffer,
                           raw_width,
                           raw_height,
                           portrait_width,
                           portrait_height,
                           label_x,
                           bounds.y + CenterOffset(bounds.height, line_height),
                           state.label_text,
                           style.role,
                           style.text_color);
        return;
    }

    // Wrap into the space left of the toggle, then centre the whole block so a one-line label
    // still sits exactly where it would have without wrapping.
    const int label_max_width = std::max(
        0, toggle_x - ClampPositive(style.control_gap) - label_x);
    std::vector<std::string> lines =
        WrapTextToWidth(style.role, std::string(state.label_text), label_max_width);
    if (lines.empty()) {
        return;
    }
    if (static_cast<int>(lines.size()) > style.max_label_lines) {
        lines.resize(static_cast<size_t>(style.max_label_lines));
    }

    const int block_height = static_cast<int>(lines.size()) * line_height;
    int line_y = bounds.y + CenterOffset(bounds.height, block_height);
    for (const std::string& line : lines) {
        DrawTypographyText(framebuffer,
                           raw_width,
                           raw_height,
                           portrait_width,
                           portrait_height,
                           label_x,
                           line_y,
                           line,
                           style.role,
                           style.text_color);
        line_y += line_height;
    }
}

}  // namespace epaper_ui
