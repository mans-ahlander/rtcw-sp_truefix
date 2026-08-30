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

/*
 * Internal load-removal state.
 *
 * A scripted level transition remains excluded from timing after the
 * physical load has completed, until the post-load UI catcher is
 * acquired and subsequently released.
 */
static qboolean speedrunTransitionPending = qfalse;
static qboolean speedrunWatchUICatcher = qfalse;
static qboolean speedrunUICatcherSeen = qfalse;

static void SV_SpeedrunUpdateLoadRemoval(void) {
	unsigned int oldFlags;
	qboolean active;

	oldFlags = g_speedrunState.flags;

	active = (g_speedrunState.flags & SR_STATE_LOADING) || speedrunTransitionPending;

	if (active) {
		g_speedrunState.flags |= SR_STATE_LOAD_REMOVAL;
	}
	else {
		g_speedrunState.flags &= ~SR_STATE_LOAD_REMOVAL;
	}

	if (oldFlags != g_speedrunState.flags) {
		SR_DEBUG(
			"SPEEDRUN: LOAD_REMOVAL %s map=%s "
			"mapSequence=%u transitionSequence=%u flags=0x%X\n",
			(g_speedrunState.flags & SR_STATE_LOAD_REMOVAL) ?
			"ON" : "OFF",
			g_speedrunState.mapName,
			g_speedrunState.mapSequence,
			g_speedrunState.transitionSequence,
			g_speedrunState.flags
		);
	}
}



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

	if (enabled) {
		g_speedrunState.flags |= flag;
	}
	else {
		g_speedrunState.flags &= ~flag;
	}

	if (flag == SR_STATE_LOADING) {
		/*
		 * An ordinary load must not inherit transition UI state.
		 * A genuine scripted transition will already have
		 * speedrunTransitionPending set.
		 */
		if (enabled && !speedrunTransitionPending) {
			speedrunWatchUICatcher = qfalse;
			speedrunUICatcherSeen = qfalse;
		}

		SV_SpeedrunUpdateLoadRemoval();
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
			"SPEEDRUN: STATE %s  mapSequence=%u  transitionSequence=%u  flags=0x%X\n",
			g_speedrunState.mapName,
			g_speedrunState.mapSequence,
			g_speedrunState.transitionSequence,
			g_speedrunState.flags
		);
	}
}

void SV_SpeedrunTransition(void) {
	g_speedrunState.transitionSequence++;

	/*
	 * A transition has been committed, but its actual map load may
	 * still be several seconds away. Do not enable LOAD_REMOVAL here.
	 */
	speedrunTransitionPending = qtrue;
	speedrunWatchUICatcher = qfalse;
	speedrunUICatcherSeen = qfalse;

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

	if (speedrunTransitionPending) {
		speedrunWatchUICatcher = qtrue;
		speedrunUICatcherSeen = qfalse;

		SR_DEBUG(
			"SPEEDRUN: UI WATCH START map=%s flags=0x%X\n",
			g_speedrunState.mapName,
			g_speedrunState.flags
		);
	}

	SR_DEBUG(
		"SPEEDRUN: NEW MAP %s mapSequence=%u "
		"transitionSequence=%u flags=0x%X\n",
		g_speedrunState.mapName,
		g_speedrunState.mapSequence,
		g_speedrunState.transitionSequence,
		g_speedrunState.flags
	);
}

void SV_SpeedrunUICatcher(qboolean active) {

	if (!speedrunTransitionPending ||
		!speedrunWatchUICatcher) {
		return;
	}

	/*
	 * Do not sample UI ownership during the physical map load.
	 *
	 * This also prevents stale catcher state from the previous map
	 * from satisfying the transition latch.
	 */
	if (g_speedrunState.flags & SR_STATE_LOADING) {
		return;
	}

	if (active) {
		if (!speedrunUICatcherSeen) {
			SR_DEBUG(
				"SPEEDRUN: UI CATCHER ACQUIRED map=%s flags=0x%X\n",
				g_speedrunState.mapName,
				g_speedrunState.flags
			);
		}

		speedrunUICatcherSeen = qtrue;
		return;
	}

	/*
	 * A false catcher state means nothing until we have first
	 * observed the post-load UI actually acquire input.
	 */
	if (!speedrunUICatcherSeen) {
		return;
	}

	SR_DEBUG(
		"SPEEDRUN: UI CATCHER RELEASED map=%s flags=0x%X\n",
		g_speedrunState.mapName,
		g_speedrunState.flags
	);

	speedrunTransitionPending = qfalse;
	speedrunWatchUICatcher = qfalse;
	speedrunUICatcherSeen = qfalse;

	SV_SpeedrunUpdateLoadRemoval();
}