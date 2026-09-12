#ifndef EPAPER_UI_GLOBAL_FOOTER_H_
#define EPAPER_UI_GLOBAL_FOOTER_H_

#include <cstdint>
#include <string>

#include "epaper_ui/overlay_geometry.h"

struct EmbeddedImageAsset;

namespace epaper_ui {

enum class GlobalFooterItemId : uint8_t {
    kNone = 0,
    kHome,
    kSettings,
    kWifi,
    kTime,
    kFolder,
    kMic,
    kSticky,
};

struct FooterButtonState {
    bool visible = false;
    bool selected = false;
    const EmbeddedImageAsset* icon = nullptr;
    // Small numeric badge overlaid on this button's icon (e.g. a pending-work count).
    bool shows_badge = false;
    std::string badge_text = {};
};

struct FooterMicState {
    bool visible = true;
    bool active = false;
    bool selected = false;
    const EmbeddedImageAsset* idle_icon = nullptr;
    const EmbeddedImageAsset* active_icon = nullptr;
};

// A small, non-focusable status glyph drawn to the left of the mic. Unlike FooterButtonState
// these are not navigable -- they report a setting, they aren't a control -- so they carry no
// selected state and no navigation role.
struct FooterStatusIndicatorState {
    bool visible = false;
    const EmbeddedImageAsset* icon = nullptr;
    // Draws a diagonal slash across the icon, the "turned off" reading. There is no dedicated
    // muted/disabled art in the asset set, so the slash is what carries the negation.
    bool slashed = true;
};

struct GlobalFooterState {
    bool visible = false;
    FooterButtonState home = {};
    FooterButtonState settings = {};
    FooterButtonState wifi = {};
    FooterButtonState time = {};
    FooterButtonState folder = {};
    FooterButtonState sticky = {};
    FooterMicState mic = {};
    // Right-hand status glyphs, laid out right-to-left immediately left of the mic.
    FooterStatusIndicatorState sound_muted = {};
    FooterStatusIndicatorState playback_off = {};
};

UiRect GlobalFooterBounds(int portrait_width, int portrait_height, const GlobalFooterState& state);
UiRect GlobalFooterItemBounds(int portrait_width,
                              int portrait_height,
                              const GlobalFooterState& state,
                              GlobalFooterItemId item);
UiRect GlobalFooterItemVisualBounds(int portrait_width,
                                    int portrait_height,
                                    const GlobalFooterState& state,
                                    GlobalFooterItemId item);
void DrawGlobalFooter(uint8_t* framebuffer,
                      int raw_width,
                      int raw_height,
                      int portrait_width,
                      int portrait_height,
                      const GlobalFooterState& state);

}  // namespace epaper_ui

#endif  // EPAPER_UI_GLOBAL_FOOTER_H_
