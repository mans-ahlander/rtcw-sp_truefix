# Changelog

## 1.43c

### Added

* Added authoritative speedrun load-removal telemetry for more precise LiveSplit timing.

  * Full map loads are detected directly by the engine.
  * Level transitions remain excluded from timing until the post-load briefing UI is dismissed.
  * Same-map quickloads are intentionally not load-removed.
* Updated the LiveSplit ASL autosplitter to use the new telemetry state instead of load-state heuristics.

### Fixed

* Fixed the Boss1 opening cutscene race that could leave the boss-room door out of sequence after skipping the cutscene.
* Fixed stale camera commands being executed across level transitions.
  * Fixed inconsistent camera state when loading affected savegames after such a case.
* Fixed a native DLL restart race where cgame could process a stale server frame after qagame had been unloaded and reloaded.
* Fixed an `AICast_QueryThink` invalid access that could occur when an AI's query target died while the AI remained in the query state.

## 1.43b

### Added

* Added **trigger brush visualization** with `g_drawTriggers 1`.

  * Displays the exact geometry of normally invisible trigger brushes.
* Added optional **trigger activation feedback** with `g_triggerFeedback 1`.

  * Plays a sound when the player activates a trigger.
* Added **player clip visualization** with `r_drawPlayerClips 1`.

  * Displays the exact geometry of invisible player-clip brushes used by the map collision system.

## 1.43a

### Added

* Added the `cvarcheck` command for validating speedrun-relevant cvars.
* Added a speedrun telemetry interface for reliable LiveSplit integration.
* Added a LiveSplit ASL autosplitter for RTCW TrueFix.

### Fixed

* Fixed an out-of-bounds access in `ResampleTexture`, based on the corrected Quake III Arena implementation.
* Fixed an out-of-bounds access in `CM_EdgePlaneNum`.
* Added additional boundary validation to the save/load system.
* Fixed two off-by-one errors in `g_save.c`.
* Added serialization for previously omitted pointer fields and introduced savegame version 19.

## 1.43

### Added

* Added top-right HUD scale, color, and shadow cvars.
* Added a speed display.
* Added an optional speed-gain indicator, protected by `sv_cheats`.
* Added `savepos` / `loadpos` commands, protected by `sv_cheats`.
* Removed cheat protection from `cg_fov` to allow its use in speedrunning.

### Fixed

* Fixed incorrect `gentities` NULL initialization in `VM_Reset`.
* Fixed an out-of-bounds access in `AICast_QueryThink`.
