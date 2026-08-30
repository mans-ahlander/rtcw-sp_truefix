# RTCW-SP TrueFix

A conservative stability and bug-fix patch for the single-player version of Return to Castle Wolfenstein (RTCW).

The primary goal of TrueFix is to improve stability and correct genuine engine and game-code defects, including crashes, memory corruption, invalid memory accesses, save/load issues, and other unsafe or unintended behavior, while preserving the original game's gameplay, physics, timing, and overall behavior as closely as possible.

Gameplay-affecting changes are intentionally avoided, even when the original behavior could reasonably be considered a bug, unless changing that behavior is necessary to correct a stability issue or other clear technical defect.

TrueFix also includes a small number of features intended specifically for RTCW speedrunning. This includes built-in speedrun telemetry designed to provide a stable interface for LiveSplit ASL autosplitters, allowing timing logic such as level transitions and load removal to be handled directly from authoritative game state rather than inferred from unrelated memory values.

## Based on

TrueFix is based on the Return to Castle Wolfenstein single-player GPL source release by id Software.

It also incorporates, adapts, or takes inspiration from selected fixes found in established community projects, most notably Knightmare's unofficial RTCW 1.42d patch, which serves as the primary foundation for this project.

Where possible, fixes are kept small and localized rather than introducing broad engine changes or unrelated modernization.

## Building

Follow the general build requirements from the original RTCW-SP source release:

https://github.com/id-Software/RTCW-SP

The Visual Studio project files included in this repository have been converted to a modern Visual Studio project format.

Open:

`src\wolf.sln`

and build the:

`wolf.vcxproj`

project.

TrueFix is intended to be built as a 32-bit Windows version of RTCW-SP.

Game data is not included in this repository. A legal installation of Return to Castle Wolfenstein is required to run the resulting binaries.

## Compatibility

TrueFix targets the original Return to Castle Wolfenstein single-player game and aims to remain compatible with the original game's data and behavior.

The project deliberately avoids large gameplay, physics, scripting, or balance changes. Its purpose is not to redesign or modernize RTCW, but to provide a more stable and predictable version of the original single-player experience.

Because TrueFix contains engine and game-code corrections, some behavior caused specifically by original bugs may differ where preserving that behavior would conflict with stability or correctness.

Savegame compatibility is preserved where practical, although fixes to invalid or inconsistent saved runtime state may necessarily change how affected savegames are restored.

## Speedrunning

TrueFix includes a small set of speedrunning-oriented additions that do not alter normal gameplay.

These include a stable telemetry interface exposed by the engine for use by external timing tools such as LiveSplit. The telemetry reports authoritative game state including map changes, level transitions, loading state, cutscenes, and dedicated load-removal state.

For supported autosplitters, this allows timing decisions to be based on explicit game state instead of fragile version-specific heuristics wherever possible.

Speedrunning-specific functionality is intended to remain isolated from normal gameplay behavior.

## Credits

* **id Software** — Return to Castle Wolfenstein and the original GPL source release

  * Reference: https://github.com/id-Software/RTCW-SP

* **Knightmare** — The unofficial RTCW 1.42d patch, containing numerous stability and compatibility fixes as well as additional features, was used as the primary foundation for TrueFix

  * Source: https://www.markshan.com/knightmare/downloads.htm

* **KoRrNiK** — RTCW speedrun patch and selected fixes incorporated or used as references

  * Source: https://github.com/KoRrNiK/RtCW-Patch_Speedrun

* **iortcw** — Selected stability and compatibility fixes were adapted from or compared against the iortcw project

  * Source: https://github.com/iortcw/iortcw

## License

The RTCW source code is distributed under the GNU General Public License version 3, subject to the additional terms accompanying the original id Software source release.

See `COPYING.txt` for the applicable license information.
