/*
===========================================================================
Practice savestate system

Cheat-only, same-map, in-memory checkpoint used for speedrun practice.
This is deliberately separate from the normal RTCW savegame system.
===========================================================================
*/

#include "g_local.h"
#include "q_shared.h"
#include "botlib.h"
#include "be_aas.h"
#include "be_ea.h"
#include "be_ai_gen.h"
#include "be_ai_goal.h"
#include "be_ai_move.h"
#include "../botai/botai.h"

#include "ai_cast.h"

typedef struct {
	qboolean valid;

	int numEntities;
	int aiMaxClients;
	int numCast;

	level_locals_t level;

	gentity_t entities[MAX_GENTITIES];
	gclient_t clients[MAX_CLIENTS];
	cast_state_t castStates[MAX_CLIENTS];
} practiceSaveState_t;

static practiceSaveState_t practiceSaveState;

/*
==================
G_ClearPracticeSaveState
==================
*/
void G_ClearPracticeSaveState(void) {
	memset(&practiceSaveState, 0, sizeof(practiceSaveState));
}

/*
==================
G_SavePracticeState
==================
*/
void G_SavePracticeState(gentity_t* ent) {
	int castCount;

	if (g_gametype.integer != GT_SINGLE_PLAYER) {
		trap_SendServerCommand(
			ent - g_entities,
			"print \"savestate is only available in single-player.\n\""
		);
		return;
	}

	if (level.num_entities < 0 ||
		level.num_entities > MAX_GENTITIES) {
		trap_SendServerCommand(
			ent - g_entities,
			"print \"Unable to create savestate: invalid entity count.\n\""
		);
		return;
	}

	if (!caststates) {
		trap_SendServerCommand(
			ent - g_entities,
			"print \"Unable to create savestate: AI state is unavailable.\n\""
		);
		return;
	}

	if (aicast_maxclients < 0 ||
		aicast_maxclients > MAX_CLIENTS) {
		trap_SendServerCommand(
			ent - g_entities,
			"print \"Unable to create savestate: invalid AI client count.\n\""
		);
		return;
	}

	/*
	 * Mark invalid while writing so a partially populated snapshot can
	 * never be restored.
	 */
	practiceSaveState.valid = qfalse;

	practiceSaveState.numEntities = level.num_entities;
	practiceSaveState.aiMaxClients = aicast_maxclients;
	practiceSaveState.numCast = numcast;

	practiceSaveState.level = level;

	memcpy(
		practiceSaveState.entities,
		g_entities,
		sizeof(practiceSaveState.entities)
	);

	memcpy(
		practiceSaveState.clients,
		level.clients,
		sizeof(practiceSaveState.clients)
	);

	memset(
		practiceSaveState.castStates,
		0,
		sizeof(practiceSaveState.castStates)
	);

	castCount = aicast_maxclients;

	memcpy(
		practiceSaveState.castStates,
		caststates,
		castCount * sizeof(cast_state_t)
	);

	practiceSaveState.valid = qtrue;

	trap_SendServerCommand(
		ent - g_entities,
		va(
			"print \"Savestate created: time %i, entities %i, cast %i.\n\"",
			practiceSaveState.level.time,
			practiceSaveState.numEntities,
			practiceSaveState.numCast
		)
	);
}


/*
==================
G_LoadPracticeState

Initial implementation only validates the stored snapshot.
Actual restoration is added separately.
==================
*/
void G_LoadPracticeState(gentity_t* ent) {
	if (!practiceSaveState.valid) {
		trap_SendServerCommand(
			ent - g_entities,
			"print \"No savestate has been created.\n\""
		);
		return;
	}

	if (practiceSaveState.numEntities < 0 ||
		practiceSaveState.numEntities > MAX_GENTITIES) {
		trap_SendServerCommand(
			ent - g_entities,
			"print \"Savestate is invalid: bad entity count.\n\""
		);
		return;
	}

	if (!caststates ||
		practiceSaveState.aiMaxClients != aicast_maxclients) {
		trap_SendServerCommand(
			ent - g_entities,
			"print \"Savestate is no longer compatible with the current AI state.\n\""
		);
		return;
	}

	trap_SendServerCommand(
		ent - g_entities,
		va(
			"print \"Savestate valid: time %i, entities %i, cast %i.\n\"",
			practiceSaveState.level.time,
			practiceSaveState.numEntities,
			practiceSaveState.numCast
		)
	);
}