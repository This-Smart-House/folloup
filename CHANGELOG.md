# Changelog

All notable changes to Followup are documented here. Format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/); versioning follows
[Semantic Versioning](docs/versioning.md). History prior to 0.2.0 is available
via `git log`, not backfilled here.

## [Unreleased]

## [0.4.0] - 2026-09-12

### Added

- A **Sound & Recording** page under Settings, with two switches that are
  remembered across reboots:
  - **Button sounds** turns off the cues played for button presses, lock and
    unlock, menus opening, startup and shutdown, and errors. The recording
    start and stop cues deliberately keep playing: their completion callbacks
    drive the recording phase machine, so suppressing them would strand a take
    mid-session, and they are also the only signal that a recording began or
    ended while the screen isn't being watched.
  - **Play back after recording** turns off the replay of a take before the tag
    menu opens. Discard is unaffected, since nothing reaches the SD card until a
    tag is chosen.
- Footer status indicators for both settings: a crossed-out icon appears to the
  left of the mic while sound or playback is switched off, and is hidden while
  they are on. These are status glyphs, not controls, so roving focus skips
  them.

### Changed

- The Settings hub now lists six headings instead of five.

## [0.3.1] - 2026-09-12

### Fixed

- Selecting a row in Todos or Notes no longer reboots the device. The
  `input_callbacks` dispatcher task ran every page callback on a 4096-byte
  stack, but page entry reads the recording archive off the SD card, and that
  FATFS/SDMMC call chain peaked at 4088 bytes — leaving no margin, so an
  interrupt arriving at depth overwrote the stack canary. FreeRTOS only checks
  that canary at a context switch, which is why the panic appeared on the next
  button press and looked like Select was at fault. The dispatcher stack is now
  8192 bytes, matching the other real-work tasks in the project.

### Added

- The boot log now reports `esp_reset_reason()`, so an unexpected reset is
  distinguishable from a deliberate restart without attaching a serial monitor
  at the right moment.

## [0.3.0] - 2026-09-11

### Changed

- Reordered the Home screen's main menu to Follow up, Todos, Notes, Topics
  (was Follow up, Topics, Notes, Todos).

## [0.2.0] - 2026-09-11

### Added

- Semantic versioning for the firmware: `version.txt` as the source of truth,
  a boot-time version log line, this changelog, and `docs/versioning.md`
  describing the workflow.

### Changed

- Baselined the starting version at 0.2.0 rather than 0.1.0, reflecting the
  substantial feature work already merged since this repo forked from
  `alxv2016/folloup-sticky` (topics, summarization, offline-transcription
  retry, settings redesign, and more — see `git log` for the full history).
