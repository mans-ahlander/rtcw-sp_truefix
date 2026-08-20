# RTCW-SP TrueFix

A conservative patch for the single-player version of Return to Castle Wolfenstein (RTCW).

The goal of this project is to improve stability and fix crashes, memory corruption, invalid accesses, 
and other genuine engine bugs while preserving the original game's gameplay and physics as closely as possible.

The project also includes a small number of features useful for RTCW speedrunning, and a built-in telemetry system for easy LiveSplit ASL script integration.

Gameplay-affecting changes are intentionally avoided, even when the original behavior may be considered a bug, unless changing it is necessary for stability.

## Based on

This project is based on the Return to Castle Wolfenstein single-player GPL source release by id Software, 
with changes derived from and inspired by community patches including Knightmare's unofficial 1.42d patch.

## Building

Follow the build instructions from the original RTCW-SP source release:

https://github.com/id-Software/RTCW-SP

The Visual Studio project files in this repository have been converted to a modern Visual Studio format. 
Open `src\wolf.sln` and build the `wolf.vcxproj` project.

Game data is not included in this repository.
A legal RTCW installation is required to run the resulting binaries.

## Compatibility

This project targets the original RTCW single-player game and is intended
to preserve the behavior of the original game as closely as possible.

## Credits

- id Software — Return to Castle Wolfenstein and the original GPL source release
    - Reference: https://github.com/id-Software/RTCW-SP
- Knightmare — The unofficial RTCW patch 1.42d with numerous stability/compatibility fixes and new features was used as a foundation for TrueFix
    - Source: https://www.markshan.com/knightmare/downloads.htm
- KoRrNiK — RTCW speedrun patch and selected fixes incorporated
    - Source: https://github.com/KoRrNiK/RtCW-Patch_Speedrun
- iortcw — Serveral stability/compatibility fixes were taken from the unofficial RTCW patch "iortcw"
    - Source: https://github.com/iortcw/iortcw

## License

The RTCW source code is distributed under the GNU General Public License
version 3, subject to the additional terms accompanying the original id
Software source release.

See `COPYING.txt` for the applicable license information.