#include "settings_sound_page_coordinator.h"

SettingsSoundPageCoordinator::SettingsSoundPageCoordinator() = default;

void SettingsSoundPageCoordinator::Show()
{
    focus_.Configure(navigation_model_.item_count, 0);
}

bool SettingsSoundPageCoordinator::MoveFocus(int delta)
{
    return focus_.Move(delta);
}

bool SettingsSoundPageCoordinator::SetFocusIndex(int index)
{
    return focus_.SetIndex(index);
}

bool SettingsSoundPageCoordinator::IsRoleFocused(page_navigation::NavigationItemRole role) const
{
    return navigation_model_.IsRoleSelected(focus_.index(), role);
}

epaper_ui::ToggleVisualState SettingsSoundPageCoordinator::BuildToggleState(bool enabled,
                                                                           bool focused)
{
    if (focused) {
        return enabled ? epaper_ui::ToggleVisualState::kFocusOn
                       : epaper_ui::ToggleVisualState::kFocusOff;
    }
    return enabled ? epaper_ui::ToggleVisualState::kOn : epaper_ui::ToggleVisualState::kOff;
}

epaper_ui::SettingsSoundPageState SettingsSoundPageCoordinator::BuildState(
    bool sound_feedback_enabled, bool review_playback_enabled) const
{
    epaper_ui::SettingsSoundPageState state = {};
    state.navigation_focus_index = focus_.index();
    state.sound_feedback_toggle = {
        .label_text = "Button sounds",
        .toggle_state = BuildToggleState(
            sound_feedback_enabled,
            IsRoleFocused(page_navigation::NavigationItemRole::kSettingsSoundFeedbackToggle)),
    };
    state.review_playback_toggle = {
        .label_text = "Play back after recording",
        .toggle_state = BuildToggleState(
            review_playback_enabled,
            IsRoleFocused(
                page_navigation::NavigationItemRole::kSettingsSoundReviewPlaybackToggle)),
    };
    state.back = {
        .label_text = "Back",
        .selected =
            IsRoleFocused(page_navigation::NavigationItemRole::kSettingsSoundBackButton),
    };
    return state;
}
