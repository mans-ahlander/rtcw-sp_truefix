#include "server.h"

// Hoyo - enable speedrun telemetry debug logging
#define SPEEDRUN_DEBUG 0

#if SPEEDRUN_DEBUG
	#define SR_DEBUG(...) Com_Printf(__VA_ARGS__)
#else
	#define SR_DEBUG(...) ((void)0)
#endif

// Hoyo - stable external interface for speedrun timers
speedrunState_t g_speedrunState = {
	SPEEDRUN_STATE_MAGIC,
	SPEEDRUN_STATE_VERSION,
	0,
	0,
	0,
};

void SV_SpeedrunSetState(unsigned int flag, qboolean enabled) {
	unsigned int oldFlags = g_speedrunState.flags;

	if (flag != SR_STATE_LEVEL_TRANSITION &&
		flag != SR_STATE_LOADING &&
		flag != SR_STATE_CUTSCENE &&
		flag != SR_STATE_PLAYER_FROZEN &&
		flag != SR_STATE_CONTROL_LOCKED) {
		SR_DEBUG(
			"WARNING: SV_SpeedrunSetState: invalid flag 0x%X\n",
			flag
		);
		return;
	}

	oldFlags = g_speedrunState.flags;
	auto test2 = &g_speedrunState;
	if (enabled) {
		g_speedrunState.flags |= flag;
	}
	else {
		g_speedrunState.flags &= ~flag;
	}

	// Only print when something actually changed.
	if (oldFlags != g_speedrunState.flags) {
		const char* name = "UNKNOWN";

		switch (flag) {
		case SR_STATE_LEVEL_TRANSITION:
			name = "LEVEL_TRANSITION";
			break;

		case SR_STATE_LOADING:
			name = "LOADING";
			break;

		case SR_STATE_CUTSCENE:
			name = "CUTSCENE";
			break;

		case SR_STATE_PLAYER_FROZEN:
			name = "PLAYER_FROZEN";
			break;

		case SR_STATE_CONTROL_LOCKED:
			name = "CONTROL_LOCKED";
			break;
		}

		// auto test = &g_speedrunState;
		SR_DEBUG(
			"SPEEDRUN: NEW MAP %s  mapSequence=%u  transitionSequence=%u  flags=0x%X\n",
			g_speedrunState.mapName,
			g_speedrunState.mapSequence,
			g_speedrunState.transitionSequence,
			g_speedrunState.flags
		);
	}
}

void SV_SpeedrunTransition(void) {
	g_speedrunState.transitionSequence++;

	SR_DEBUG(
		"SPEEDRUN: TRANSITION transitionSequence=%u "
		"mapSequence=%u flags=0x%X\n",
		g_speedrunState.transitionSequence,
		g_speedrunState.mapSequence,
		g_speedrunState.flags
	);
}

void SV_SpeedrunNewMap(const char* mapName) {
	Q_strncpyz(
		g_speedrunState.mapName,
		mapName,
		sizeof(g_speedrunState.mapName)
	);

	g_speedrunState.mapSequence++;

	SR_DEBUG(
		"SPEEDRUN: NEW MAP %s mapSequence=%u "
		"transitionSequence=%u flags=0x%X\n",
		g_speedrunState.mapName,
		g_speedrunState.mapSequence,
		g_speedrunState.transitionSequence,
		g_speedrunState.flags
	);
}