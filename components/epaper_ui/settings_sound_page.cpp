#include "epaper_ui/settings_sound_page.h"

#include <algorithm>

#include "render_utils.h"

namespace epaper_ui {
namespace {

constexpr auto kTitleRole = design::TypographyRole::kHeadingH1;
constexpr int kSideInset = design::spacing::k16;
constexpr int kTopGap = design::spacing::k24;
constexpr int kHeadingBottomPadding = design::spacing::k24;
constexpr int kBackButtonGap = design::spacing::k24;

struct Layout {
    UiRect sound_feedback_toggle = {};
    UiRect review_playback_toggle = {};
    UiRect back = {};
};

MenuToggleStyle SoundToggleStyle(int page_width)
{
    MenuToggleStyle style = {};
    style.width = page_width;
    return style;
}

// "Play back after recording" is wider than the space left of the toggle at the default label
// role, and MenuToggle does not clip, so on one line it runs underneath the switch. Give this
// row two lines and the height to hold them.
MenuToggleStyle ReviewToggleStyle(int page_width)
{
    MenuToggleStyle style = {};
    style.width = page_width;
    style.max_label_lines = 2;
    style.height = std::max(design::menu_toggle::kHeight,
                            (2 * LineHeight(style.role)) + design::spacing::k16);
    return style;
}

Layout BuildLayout(int portrait_width, int portrait_height, const SettingsSoundPageState& state)
{
    (void)portrait_height;
    (void)state;
    const int page_x = kSideInset;
    const int page_width = std::max(0, portrait_width - (2 * kSideInset));
    const int title_y = StatusBarHeight() + kTopGap;
    const int title_bottom = title_y + LineHeight(kTitleRole);

    // The two toggles stack with no gap, matching the Wi-Fi page: the bottom border on each
    // MenuToggle is what separates them, so an added gap would read as a broken divider.
    MenuToggleStyle sound_style = SoundToggleStyle(page_width);
    MenuToggleStyle review_style = ReviewToggleStyle(page_width);

    const UiRect sound_feedback_toggle =
        MenuToggleBounds(page_x, title_bottom + kHeadingBottomPadding, sound_style);
    const UiRect review_playback_toggle =
        MenuToggleBounds(page_x, sound_feedback_toggle.bottom(), review_style);

    ButtonStyle back_style = {};
    back_style.width = page_width;
    back_style.center_label = true;
    const UiRect back = ButtonBounds(page_x, review_playback_toggle.bottom() + kBackButtonGap,
                                     state.back, back_style);

    return {
        .sound_feedback_toggle = sound_feedback_toggle,
        .review_playback_toggle = review_playback_toggle,
        .back = back,
    };
}

}  // namespace

UiRect SettingsSoundPageItemBounds(int portrait_width,
                                   int portrait_height,
                                   const SettingsSoundPageState& state,
                                   SettingsSoundPageItemId item)
{
    const Layout layout = BuildLayout(portrait_width, portrait_height, state);
    switch (item) {
        case SettingsSoundPageItemId::kSoundFeedbackToggle:
            return layout.sound_feedback_toggle;
        case SettingsSoundPageItemId::kReviewPlaybackToggle:
            return layout.review_playback_toggle;
        case SettingsSoundPageItemId::kBack:
            return layout.back;
        case SettingsSoundPageItemId::kNone:
        default:
            return {};
    }
}

bool HitTestSettingsSoundPageItem(int portrait_width,
                                  int portrait_height,
                                  const SettingsSoundPageState& state,
                                  int x,
                                  int y,
                                  SettingsSoundPageItemId* item)
{
    if (item != nullptr) {
        *item = SettingsSoundPageItemId::kNone;
    }

    constexpr SettingsSoundPageItemId kItems[] = {
        SettingsSoundPageItemId::kSoundFeedbackToggle,
        SettingsSoundPageItemId::kReviewPlaybackToggle,
        SettingsSoundPageItemId::kBack,
    };
    for (SettingsSoundPageItemId candidate : kItems) {
        const UiRect bounds =
            SettingsSoundPageItemBounds(portrait_width, portrait_height, state, candidate);
        if (!bounds.IsEmpty() && bounds.Contains(x, y)) {
            if (item != nullptr) {
                *item = candidate;
            }
            return true;
        }
    }
    return false;
}

void DrawSettingsSoundPage(uint8_t* framebuffer,
                           int raw_width,
                           int raw_height,
                           int portrait_width,
                           int portrait_height,
                           const SettingsSoundPageState& state,
                           const StatusBarState& status_bar_state,
                           const GlobalFooterState& footer_state)
{
    if (framebuffer == nullptr) {
        return;
    }

    FillPortraitRect(framebuffer, raw_width, raw_height, portrait_width, portrait_height,
                     {0, 0, portrait_width, portrait_height}, design::color::kWhite);
    DrawStatusBar(framebuffer, raw_width, raw_height, portrait_width, portrait_height,
                  status_bar_state);

    const Layout layout = BuildLayout(portrait_width, portrait_height, state);
    const int title_x = kSideInset;
    const int title_y = StatusBarHeight() + kTopGap;
    DrawTypographyText(framebuffer, raw_width, raw_height, portrait_width, portrait_height,
                       title_x, title_y, state.title_text, kTitleRole, design::color::kBlack);

    DrawMenuToggle(framebuffer, raw_width, raw_height, portrait_width, portrait_height,
                   layout.sound_feedback_toggle.x, layout.sound_feedback_toggle.y,
                   state.sound_feedback_toggle,
                   SoundToggleStyle(layout.sound_feedback_toggle.width));

    DrawMenuToggle(framebuffer, raw_width, raw_height, portrait_width, portrait_height,
                   layout.review_playback_toggle.x, layout.review_playback_toggle.y,
                   state.review_playback_toggle,
                   ReviewToggleStyle(layout.review_playback_toggle.width));

    DrawButton(framebuffer, raw_width, raw_height, portrait_width, portrait_height, layout.back.x,
               layout.back.y, state.back,
               {.width = layout.back.width, .center_label = true});

    DrawGlobalFooter(framebuffer, raw_width, raw_height, portrait_width, portrait_height,
                     footer_state);
}

}  // namespace epaper_ui
