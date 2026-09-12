#include "settings_sound_page_interactions.h"

namespace settings_sound_page_interactions {
namespace {

using page_navigation::NavigationItemRole;

}  // namespace

ActivateResult HandlePrimaryActivate(const SettingsSoundPageCoordinator& coordinator)
{
    ActivateResult result = {};

    if (coordinator.IsRoleFocused(NavigationItemRole::kSettingsSoundFeedbackToggle)) {
        result.handled = true;
        // Deliberately still requests the cue: it is the last thing heard when switching sound
        // off, and the confirmation that the press registered when switching it back on.
        result.play_activate_cue = true;
        result.intent = ActivateIntent::kToggleSoundFeedback;
        return result;
    }
    if (coordinator.IsRoleFocused(NavigationItemRole::kSettingsSoundReviewPlaybackToggle)) {
        result.handled = true;
        result.play_activate_cue = true;
        result.intent = ActivateIntent::kToggleReviewPlayback;
        return result;
    }
    if (coordinator.IsRoleFocused(NavigationItemRole::kSettingsSoundBackButton)) {
        // Returns to the Settings hub -- this page is only ever reached from there.
        result.handled = true;
        result.play_activate_cue = true;
        result.intent = ActivateIntent::kShowSettings;
        return result;
    }
    if (coordinator.IsRoleFocused(NavigationItemRole::kFooterHome)) {
        result.handled = true;
        result.play_activate_cue = true;
        result.intent = ActivateIntent::kShowHome;
        return result;
    }

    return result;
}

void ApplyPrimaryActivateResult(const ActivateResult& result, const ActivateCallbacks& callbacks)
{
    switch (result.intent) {
        case ActivateIntent::kShowHome:
            if (callbacks.show_home) {
                callbacks.show_home();
            }
            break;
        case ActivateIntent::kShowSettings:
            if (callbacks.show_settings) {
                callbacks.show_settings();
            }
            break;
        case ActivateIntent::kToggleSoundFeedback:
            if (callbacks.toggle_sound_feedback) {
                callbacks.toggle_sound_feedback();
            }
            break;
        case ActivateIntent::kToggleReviewPlayback:
            if (callbacks.toggle_review_playback) {
                callbacks.toggle_review_playback();
            }
            break;
        case ActivateIntent::kNone:
        default:
            break;
    }
}

FocusMoveResult HandleMoveFocus(SettingsSoundPageCoordinator& coordinator, int delta)
{
    FocusMoveResult result = {};
    if (!coordinator.MoveFocus(delta)) {
        return result;
    }
    result.handled = true;
    result.play_navigation_cue = true;
    result.apply_page_state = true;
    return result;
}

}  // namespace settings_sound_page_interactions
