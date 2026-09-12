#ifndef EPAPER_UI_SETTINGS_SOUND_PAGE_H_
#define EPAPER_UI_SETTINGS_SOUND_PAGE_H_

#include <cstdint>
#include <string_view>

#include "epaper_ui/button.h"
#include "epaper_ui/global_footer.h"
#include "epaper_ui/menu_toggle.h"
#include "epaper_ui/status_bar.h"

namespace epaper_ui {

enum class SettingsSoundPageItemId : uint8_t {
    kNone = 0,
    kSoundFeedbackToggle,
    kReviewPlaybackToggle,
    kBack,
};

struct SettingsSoundPageState {
    int navigation_focus_index = -1;
    std::string_view title_text = "Sound & Recording";
    // Button/status cues. Does not cover the recording start/stop cues, which sequence the
    // recording phase machine -- see feedback_service.h.
    MenuToggleState sound_feedback_toggle = {};
    // Whether a finished take is replayed before the tag menu opens.
    MenuToggleState review_playback_toggle = {};
    // Returns to the Settings hub -- this page is reached only from there.
    ButtonState back = {};
};

UiRect SettingsSoundPageItemBounds(int portrait_width,
                                   int portrait_height,
                                   const SettingsSoundPageState& state,
                                   SettingsSoundPageItemId item);
bool HitTestSettingsSoundPageItem(int portrait_width,
                                  int portrait_height,
                                  const SettingsSoundPageState& state,
                                  int x,
                                  int y,
                                  SettingsSoundPageItemId* item);
void DrawSettingsSoundPage(uint8_t* framebuffer,
                           int raw_width,
                           int raw_height,
                           int portrait_width,
                           int portrait_height,
                           const SettingsSoundPageState& state,
                           const StatusBarState& status_bar_state,
                           const GlobalFooterState& footer_state);

}  // namespace epaper_ui

#endif  // EPAPER_UI_SETTINGS_SOUND_PAGE_H_
