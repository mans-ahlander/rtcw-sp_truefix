# Changelog

## 1.43b

### Added
- Added **trigger brush visualization** with `g_drawTriggers 1`.
  - Displays the exact geometry of normally invisible trigger brushes.
- Added optional **trigger activation feedback** with `g_triggerFeedback 1`.
  - Plays a sound when the activates a trigger.
- Added **player clip visualization** with `r_drawPlayerClip 1`.
  - Displays the exact geometry of invisible player clip brushes used by the map collision system.

## 1.43a

### Added
- Added the `cvarcheck` command for validating speedruns.
- Added a speedrun telemetry system for reliable LiveSplit integration.
- Added a LiveSplit ASL autosplitter for RTCW TrueFix.

### Fixed
- Fixed an out-of-bounds access in `ResampleTexture`, matching the latest Quake III Arena implementation.
- Fixed an out-of-bounds access in `CM_EdgePlaneNum`.
- Added additional boundary safeguards to the save/load system.
- Fixed two off-by-one errors in `g_save.c`.
- Added serialization for previously omitted pointer fields and introduced save version 19.

## 1.43

### Added
- Added top-right HUD scale, color, and shadow cvars.
- Added a speed display.
- Added an optional speed-gain indicator, protected by `sv_cheats`.
- Added `savepos` / `loadpos` commands, protected by `sv_cheats`.
- Removed cheat protection from `cg_fov`.

### Fixed
- Fixed incorrect `gentities` NULL initialization in `VM_Reset`.
- Fixed an out-of-bounds access in `AICast_QueryThink`.