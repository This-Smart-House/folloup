#include "settings_sound_page_runtime.h"

#include <mutex>

#include "esp_log.h"
#include "feedback_service.h"
#include "page_navigation/navigation_model.h"
#include "page_navigation/page_focus_projection.h"
#include "recording_session_service.h"
#include "settings_sound_page_coordinator.h"
#include "settings_sound_page_interactions.h"
#include "ui_refresh_runtime.h"

namespace settings_sound_page_runtime {
namespace {

constexpr const char* kTag = "SettingsSoundPage";

std::mutex s_mutex;
SettingsSoundPageCoordinator s_coordinator = {};

footer_runtime::FooterFocusItem FooterItemForSelectedIndex(int selected_index)
{
    switch (selected_index) {
        case 1:
            return footer_runtime::FooterFocusItem::kSettings;
        case 2:
            return footer_runtime::FooterFocusItem::kWifi;
        case 3:
            return footer_runtime::FooterFocusItem::kTime;
        case 0:
            return footer_runtime::FooterFocusItem::kHome;
        case 4:
            return footer_runtime::FooterFocusItem::kSticky;
        default:
            return footer_runtime::FooterFocusItem::kNone;
    }
}

page_navigation::NavigationItemRole FooterRoleForFooterItem(footer_runtime::FooterFocusItem item)
{
    switch (item) {
        case footer_runtime::FooterFocusItem::kSettings:
            return page_navigation::NavigationItemRole::kFooterSettings;
        case footer_runtime::FooterFocusItem::kWifi:
            return page_navigation::NavigationItemRole::kFooterWifi;
        case footer_runtime::FooterFocusItem::kHome:
            return page_navigation::NavigationItemRole::kFooterHome;
        case footer_runtime::FooterFocusItem::kTime:
            return page_navigation::NavigationItemRole::kFooterTime;
        case footer_runtime::FooterFocusItem::kSticky:
            return page_navigation::NavigationItemRole::kFooterSticky;
        case footer_runtime::FooterFocusItem::kNone:
        case footer_runtime::FooterFocusItem::kFolder:
        case footer_runtime::FooterFocusItem::kMic:
        default:
            return page_navigation::NavigationItemRole::kUnknown;
    }
}

footer_runtime::ProjectionState BuildFooterProjectionStateLocked()
{
    const page_navigation::PageFocusProjection projection = page_navigation::ProjectPageFocus(
        s_coordinator.navigation_model(), page_navigation::NavigationItemSection::kNone,
        s_coordinator.focus().index(), -1, -1);
    footer_runtime::ProjectionState state = {};
    state.focused_item = FooterItemForSelectedIndex(projection.footer_selected_index);
    return state;
}

bool FooterProjectionChangedForFocusIndexes(int old_focus_index, int new_focus_index)
{
    const page_navigation::PageFocusProjection old_projection = page_navigation::ProjectPageFocus(
        s_coordinator.navigation_model(), page_navigation::NavigationItemSection::kNone,
        old_focus_index, -1, -1);
    const page_navigation::PageFocusProjection new_projection = page_navigation::ProjectPageFocus(
        s_coordinator.navigation_model(), page_navigation::NavigationItemSection::kNone,
        new_focus_index, -1, -1);
    return FooterItemForSelectedIndex(old_projection.footer_selected_index) !=
           FooterItemForSelectedIndex(new_projection.footer_selected_index);
}

epaper_ui::SettingsSoundPageState BuildStateLocked()
{
    // Read straight from the owning services rather than caching a copy here, so the row always
    // reflects what actually persisted -- including a write that failed.
    return s_coordinator.BuildState(feedback_service::IsSoundEnabled(),
                                    recording_session_service::IsReviewPlaybackEnabled());
}

}  // namespace

esp_err_t UpdateDisplayState()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    return display_service::SetSettingsSoundPageState(BuildStateLocked());
}

esp_err_t UpdateDisplayStateAndRequestRefresh(display_service::RefreshMode refresh_mode)
{
    return UpdateDisplayStateAndRequestRefresh(
        display_service::RefreshRequest{.refresh_mode = refresh_mode});
}

esp_err_t UpdateDisplayStateAndRequestRefresh(
    const display_service::RefreshRequest& refresh_request)
{
    return ui_refresh_runtime::Schedule(ui_refresh_runtime::SurfaceKey::kSettingsSoundPage,
                                        &UpdateDisplayState, refresh_request);
}

page_actions::FocusMoveOutcome MoveFocus(int delta)
{
    page_actions::FocusMoveOutcome result = {};
    int old_focus_index = -1;
    int new_focus_index = -1;
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        old_focus_index = s_coordinator.focus().index();
        result = settings_sound_page_interactions::HandleMoveFocus(s_coordinator, delta);
        if (!result.handled) {
            return result;
        }
        new_focus_index = s_coordinator.focus().index();
    }

    result.sync_footer_projection =
        FooterProjectionChangedForFocusIndexes(old_focus_index, new_focus_index);
    return result;
}

settings_sound_page_interactions::ActivateResult ActivateFocusedItem()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    return settings_sound_page_interactions::HandlePrimaryActivate(s_coordinator);
}

footer_runtime::ProjectionState BuildFooterProjectionState()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    return BuildFooterProjectionStateLocked();
}

page_actions::FocusUpdateOutcome FocusFooterItem(footer_runtime::FooterFocusItem item)
{
    page_actions::FocusUpdateOutcome result = {};
    const page_navigation::NavigationItemRole role = FooterRoleForFooterItem(item);
    if (role == page_navigation::NavigationItemRole::kUnknown) {
        return result;
    }

    int old_focus_index = -1;
    int new_focus_index = -1;
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        const int focus_index = s_coordinator.navigation_model().IndexOfRole(role);
        if (focus_index < 0) {
            return result;
        }
        old_focus_index = s_coordinator.focus().index();
        if (!s_coordinator.SetFocusIndex(focus_index)) {
            return result;
        }
        new_focus_index = s_coordinator.focus().index();
    }

    result.handled = true;
    result.apply_page_state = true;
    result.sync_footer_projection =
        FooterProjectionChangedForFocusIndexes(old_focus_index, new_focus_index);
    return result;
}

void ResetFocus()
{
    footer_runtime::ProjectionState projection = {};
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_coordinator.Show();
        projection = BuildFooterProjectionStateLocked();
    }
    footer_runtime::SetProjectionState(projection);
}

namespace {

// Each toggle also drives a footer indicator, and the footer is drawn as part of this page's
// own render pass -- so refreshing the footer's state before scheduling the page repaint gets
// both in a single refresh rather than two.
esp_err_t RepaintPageAndFooter()
{
    const esp_err_t footer_err = footer_runtime::UpdateDisplayState();
    if (footer_err != ESP_OK && footer_err != ESP_ERR_INVALID_STATE) {
        ESP_LOGW(kTag, "Footer sync after toggle failed: %s", esp_err_to_name(footer_err));
    }
    return UpdateDisplayStateAndRequestRefresh(display_service::RefreshMode::kPartial);
}

}  // namespace

esp_err_t ToggleSoundFeedback()
{
    const bool next = !feedback_service::IsSoundEnabled();
    const esp_err_t err = feedback_service::SetSoundEnabled(next);
    if (err != ESP_OK) {
        ESP_LOGW(kTag, "Sound feedback toggle failed: %s", esp_err_to_name(err));
    }
    // Repaint either way: on failure the row redraws at the unchanged value rather than showing
    // a state that was never stored.
    return RepaintPageAndFooter();
}

esp_err_t ToggleReviewPlayback()
{
    const bool next = !recording_session_service::IsReviewPlaybackEnabled();
    const esp_err_t err = recording_session_service::SetReviewPlaybackEnabled(next);
    if (err != ESP_OK) {
        ESP_LOGW(kTag, "Review playback toggle failed: %s", esp_err_to_name(err));
    }
    return RepaintPageAndFooter();
}

}  // namespace settings_sound_page_runtime
