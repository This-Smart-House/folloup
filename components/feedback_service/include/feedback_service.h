#ifndef FEEDBACK_SERVICE_H_
#define FEEDBACK_SERVICE_H_

#include "esp_err.h"

namespace feedback_service {

enum class FeedbackEvent {
    kStartup,
    kGeminiConnected,
    kLock,
    kUnlock,
    kRecordingStart,
    kModalOpen,
    kButtonClick,
    kButtonDoubleClick,
    kButtonLongPress,
    kShutdown,
    kError,
};

esp_err_t Init();
esp_err_t Play(FeedbackEvent event);

// Sound feedback for app events (button presses, lock/unlock, modal open, startup/shutdown,
// errors) can be turned off from Settings -> Sound & Recording. Persisted in NVS and cached in
// memory, because Play() runs on the input path and must not touch NVS per button press.
//
// This does NOT silence the recording start/stop cues: those are played directly by
// recording_session_service and their completion callbacks drive the recording phase machine,
// so suppressing them would strand a session mid-take. See the comment on Play().
bool IsSoundEnabled();
esp_err_t SetSoundEnabled(bool enabled);

}  // namespace feedback_service

#endif  // FEEDBACK_SERVICE_H_
