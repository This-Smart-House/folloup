#include "feedback_service.h"

#include <atomic>

#include "esp_log.h"
#include "nvs.h"
#include "waveshare_board.h"
#include "system_sound_service.h"

namespace feedback_service {
namespace {

constexpr const char* kTag = "FeedbackService";
constexpr const char* kNvsNamespace = "feedback";
constexpr const char* kSoundEnabledKey = "sound_on";

// Cached so Play() -- which runs on the input path, once per button press -- never opens NVS.
// Defaults to on, both for a fresh device and if the read fails.
std::atomic<bool> s_sound_enabled{true};

bool LoadSoundEnabledFromNvs()
{
    nvs_handle_t handle = 0;
    if (nvs_open(kNvsNamespace, NVS_READONLY, &handle) != ESP_OK) {
        // No namespace yet: first boot, or never toggled. Sound stays on.
        return true;
    }
    uint8_t enabled = 1;
    const esp_err_t err = nvs_get_u8(handle, kSoundEnabledKey, &enabled);
    nvs_close(handle);
    if (err != ESP_OK) {
        return true;
    }
    return enabled != 0;
}

// Maps a feedback event onto the closest system-sound cue (played through the
// ES8311 codec). There is no dedicated buzzer on this board.
SoundCue CueForEvent(FeedbackEvent event)
{
    switch (event) {
        case FeedbackEvent::kStartup:
            return SoundCue::kStartup;
        case FeedbackEvent::kGeminiConnected:
            return SoundCue::kOnline;
        case FeedbackEvent::kLock:
            return SoundCue::kLock;
        case FeedbackEvent::kUnlock:
            return SoundCue::kUnlock;
        case FeedbackEvent::kRecordingStart:
            return SoundCue::kButtonActivate;
        case FeedbackEvent::kModalOpen:
            return SoundCue::kModalNotification;
        case FeedbackEvent::kButtonClick:
        case FeedbackEvent::kButtonDoubleClick:
        case FeedbackEvent::kButtonLongPress:
            return SoundCue::kButtonActivate;
        case FeedbackEvent::kShutdown:
            return SoundCue::kModalNotification;
        case FeedbackEvent::kError:
        default:
            return SoundCue::kInterrupt;
    }
}

}  // namespace

esp_err_t Init()
{
    AudioCodec* codec = waveshare_board::GetAudioCodec();
    if (codec == nullptr) {
        ESP_LOGW(kTag, "Audio codec unavailable; feedback cues disabled");
        return ESP_ERR_NOT_FOUND;
    }

    // Starts the sound-service playback task and warms (decodes) the cue cache. The cache is
    // warmed even when sound is off, so toggling it back on doesn't pay a first-cue decode stall.
    SystemSoundService::GetInstance().Initialize(codec);
    s_sound_enabled.store(LoadSoundEnabledFromNvs(), std::memory_order_relaxed);
    ESP_LOGI(kTag, "Feedback service initialized (audio cues via ES8311, sound %s)",
             s_sound_enabled.load(std::memory_order_relaxed) ? "on" : "off");
    return ESP_OK;
}

esp_err_t Play(FeedbackEvent event)
{
    // Muting here covers every app-level cue, because this function has exactly one call site
    // (app_shell's event->cue mapping). It deliberately does not reach the recording start/stop
    // cues, which recording_session_service plays directly: those carry completion callbacks that
    // advance the phase machine, so not playing them would leave a take stuck in kStartCue.
    if (!s_sound_enabled.load(std::memory_order_relaxed)) {
        return ESP_OK;
    }
    SystemSoundService::GetInstance().PlayCue(CueForEvent(event));
    return ESP_OK;
}

bool IsSoundEnabled()
{
    return s_sound_enabled.load(std::memory_order_relaxed);
}

esp_err_t SetSoundEnabled(bool enabled)
{
    nvs_handle_t handle = 0;
    if (nvs_open(kNvsNamespace, NVS_READWRITE, &handle) != ESP_OK) {
        ESP_LOGW(kTag, "Sound setting: nvs_open failed");
        return ESP_FAIL;
    }
    esp_err_t err = nvs_set_u8(handle, kSoundEnabledKey, enabled ? 1 : 0);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    if (err != ESP_OK) {
        ESP_LOGW(kTag, "Sound setting: persist failed: %s", esp_err_to_name(err));
        return err;
    }
    s_sound_enabled.store(enabled, std::memory_order_relaxed);
    ESP_LOGI(kTag, "Sound feedback %s", enabled ? "enabled" : "disabled");
    return ESP_OK;
}

}  // namespace feedback_service
