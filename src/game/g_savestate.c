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

	qboolean castActive[MAX_CLIENTS];
	qboolean botStatePresent[MAX_CLIENTS];
	bot_state_t botStates[MAX_CLIENTS];

	gentity_t* camEnt;

	int wolfKickTimer;

	/* Runtime state outside the main level/entity arrays. */
	int gReloading;
	qboolean saveGamePending;
	char screenFade[MAX_STRING_CHARS];
} practiceSaveState_t;

static practiceSaveState_t practiceSaveState;

static void G_SanitizePracticeSaveState(void);
static qboolean G_PracticeStateAICompatible(void);
static void G_RestorePracticeEntity(int index);
static void G_RestorePracticeCastState(int index);

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
qboolean G_SavePracticeState(gentity_t* ent) {
	int castCount;
	int i;

	if (!g_cheats.integer) {
		trap_SendServerCommand(
			ent - g_entities,
			"print \"Cheats are not enabled on this server.\n\""
		);
		return qfalse;
	}

	if (ent->health <= 0) {
		trap_SendServerCommand(
			ent - g_entities,
			"print \"You must be alive to create a savestate.\n\""
		);
		return qfalse;
	}

	if (g_gametype.integer != GT_SINGLE_PLAYER) {
		trap_SendServerCommand(
			ent - g_entities,
			"print \"savestate is only available in single-player.\n\""
		);
		return qfalse;
	}

	if (level.num_entities < 0 ||
		level.num_entities > MAX_GENTITIES) {
		trap_SendServerCommand(
			ent - g_entities,
			"print \"Unable to create savestate: invalid entity count.\n\""
		);
		return qfalse;
	}

	if (!caststates) {
		trap_SendServerCommand(
			ent - g_entities,
			"print \"Unable to create savestate: AI state is unavailable.\n\""
		);
		return qfalse;
	}

	if (aicast_maxclients < 0 ||
		aicast_maxclients > MAX_CLIENTS) {
		trap_SendServerCommand(
			ent - g_entities,
			"print \"Unable to create savestate: invalid AI client count.\n\""
		);
		return qfalse;
	}

	/*
	 * Mark invalid while writing so a partially populated snapshot can never be restored.
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

	practiceSaveState.camEnt = g_camEnt;

	for (i = 0; i < aicast_maxclients; i++) {
		practiceSaveState.castActive[i] =
			(caststates[i].bs && caststates[i].bs->inuse)
			? qtrue
			: qfalse;

		if (botstates[i]) {
			practiceSaveState.botStatePresent[i] = qtrue;
			practiceSaveState.botStates[i] = *botstates[i];
		}
		else {
			practiceSaveState.botStatePresent[i] = qfalse;
		}
	}

	practiceSaveState.wolfKickTimer = G_GetWolfKickTimer();

	G_SanitizePracticeSaveState();

	practiceSaveState.gReloading = g_reloading.integer;
	practiceSaveState.saveGamePending = saveGamePending;

	trap_GetConfigstring(
		CS_SCREENFADE,
		practiceSaveState.screenFade,
		sizeof(practiceSaveState.screenFade)
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

	return qtrue;
}


/*
==================
G_LoadPracticeState
==================
*/
qboolean G_LoadPracticeState(gentity_t* ent) {
	int i;
	int restoreCount;
	int currentNumEntities;

	if (!g_cheats.integer) {
		trap_SendServerCommand(
			ent - g_entities,
			"print \"Cheats are not enabled on this server.\n\""
		);
		return qfalse;
	}

	if (!practiceSaveState.valid) {
		trap_SendServerCommand(
			ent - g_entities,
			"print \"No savestate has been created.\n\""
		);
		return qfalse;
	}

	if (practiceSaveState.numEntities < 0 ||
		practiceSaveState.numEntities > MAX_GENTITIES) {

		trap_SendServerCommand(
			ent - g_entities,
			"print \"Savestate is invalid: bad entity count.\n\""
		);
		return qfalse;
	}

	if (!G_PracticeStateAICompatible()) {
		trap_SendServerCommand(
			ent - g_entities,
			"print \"Savestate cannot be restored: AI client layout changed.\n\""
		);
		return qfalse;
	}

	currentNumEntities = level.num_entities;

	restoreCount = currentNumEntities;

	if (practiceSaveState.numEntities > restoreCount) {
		restoreCount = practiceSaveState.numEntities;
	}

	/*
	 * From this point onward, the restore is atomic with respect to
	 * normal game simulation. No GAME_RUN_FRAME can occur until this
	 * command returns.
	 *
	 * Rewind the engine first, then immediately restore the matching
	 * game-side timeline.
	 */
	if (!trap_PracticeRewind(practiceSaveState.level.time)) {
		trap_SendServerCommand(
			ent - g_entities,
			"print \"Unable to rewind server for savestate.\n\""
		);
		return qfalse;
	}

	/*
	 * Restore per-level and client state.
	 */
	level = practiceSaveState.level;

	memcpy(
		g_clients,
		practiceSaveState.clients,
		sizeof(practiceSaveState.clients)
	);

	/*
	 * Explicitly restore the canonical array pointers even though their
	 * addresses should be unchanged in the same VM.
	 */
	level.clients = g_clients;
	level.gentities = g_entities;
	level.gentitySize = sizeof(gentity_t);

	/*
	 * Restore entities and synchronize server collision/AAS state.
	 */
	for (i = 0; i < restoreCount; i++) {
		G_RestorePracticeEntity(i);
	}

	/*
	 * level.num_entities may have gone backwards. Tell the engine the
	 * authoritative entity count again.
	 */
	trap_LocateGameData(
		level.gentities,
		level.num_entities,
		sizeof(gentity_t),
		&level.clients[0].ps,
		sizeof(level.clients[0])
	);

	/*
	 * Entity tag connections should be repaired only after all entities
	 * have been restored.
	 */
	for (i = 0; i < level.num_entities; i++) {
		if (!g_entities[i].inuse) {
			continue;
		}

		if (g_entities[i].tagName &&
			g_entities[i].tagParent) {

			G_ProcessTagConnect(&g_entities[i], qfalse);
		}
	}

	/*
	 * Restore Cast AI and its game-side bot state.
	 */
	for (i = 0; i < aicast_maxclients; i++) {
		G_RestorePracticeCastState(i);
	}

	numcast = practiceSaveState.numCast;

	G_SetWolfKickTimer(
		practiceSaveState.wolfKickTimer
	);

	/*
	 * Restore runtime state that lives outside level/gentity/gclient.
	 */
	saveGamePending = practiceSaveState.saveGamePending;

	trap_Cvar_Set(
		"g_reloading",
		va("%i", practiceSaveState.gReloading)
	);
	trap_Cvar_Update(&g_reloading);

	/*
	* Restore the checkpoint's screen fade state. This also cancels a
	* pending death fade when the checkpoint was normal gameplay.
	*/
	trap_SetConfigstring(
		CS_SCREENFADE,
		practiceSaveState.screenFade
	);

	if (!practiceSaveState.gReloading) {
		trap_SendServerCommand(
			0,
			"snd_fade 1 0"
		);
	}

	/*
	 * Restore the global script-camera pointer and repair any impossible
	 * camera combination.
	 */
	g_camEnt = practiceSaveState.camEnt;
	G_ValidatePlayerCameraState();

	/*
	 * Function-local AI timing survived because qagame itself was not
	 * restarted. Rebase it to the restored timeline.
	 */
	AICast_ResetFrameTiming(level.time);

	trap_SendServerCommand(
		0,
		va(
			"print \"Savestate restored: time %i, entities %i, cast %i.\n\"",
			level.time,
			level.num_entities,
			numcast
		)
	);

	return qtrue;
}

static qboolean G_PracticeStateAICompatible(void) {
	int i;
	qboolean active;
	qboolean present;

	if (!caststates ||
		practiceSaveState.aiMaxClients != aicast_maxclients ||
		practiceSaveState.numCast != numcast) {
		return qfalse;
	}

	for (i = 0; i < aicast_maxclients; i++) {
		active =
			(caststates[i].bs && caststates[i].bs->inuse)
			? qtrue
			: qfalse;

		if (active != practiceSaveState.castActive[i]) {
			return qfalse;
		}

		present = botstates[i] ? qtrue : qfalse;

		if (present != practiceSaveState.botStatePresent[i]) {
			return qfalse;
		}
	}

	return qtrue;
}

static void G_SanitizePracticeSaveState(void) {
	int i;
	gclient_t* cl;
	gentity_t* ent;

	for (i = 0; i < MAX_CLIENTS; i++) {
		cl = &practiceSaveState.clients[i];

		memset(cl->ps.events, 0, sizeof(cl->ps.events));
		memset(cl->ps.eventParms, 0, sizeof(cl->ps.eventParms));

		cl->ps.eventSequence = 0;
		cl->ps.oldEventSequence = 0;
		cl->ps.entityEventSequence = 0;
	}

	for (i = 0; i < MAX_GENTITIES; i++) {
		ent = &practiceSaveState.entities[i];

		if (!ent->freeAfterEvent) {
			ent->s.event = 0;
			memset(ent->s.events, 0, sizeof(ent->s.events));
			memset(ent->s.eventParms, 0, sizeof(ent->s.eventParms));
			ent->s.eventSequence = 0;
			ent->eventTime = 0;
		}
	}
}


static void G_RestorePracticeEntity(int index) {
	gentity_t backup;
	gentity_t backup2;
	gentity_t* ent;
	qboolean shouldLink;

	ent = &g_entities[index];

	/*
	 * Keep the current version long enough to undo engine-side state
	 * and to determine whether mover portal state changed.
	 */
	backup = *ent;

	if (ent->AASblocking) {
		G_SetAASBlockingEntity(ent, qfalse);
	}

	if (ent->r.linked) {
		trap_UnlinkEntity(ent);
	}

	/*
	 * Restore the checkpoint version.
	 */
	*ent = practiceSaveState.entities[index];

	/*
	 * Anything above the checkpoint's high-water entity count must
	 * remain absent after the rewind.
	 */
	if (index >= practiceSaveState.numEntities) {
		ent->r.linked = qfalse;
		return;
	}

	shouldLink = ent->r.linked;

	/*
	 * trap_UnlinkEntity() acted on the old object. After memcpy the
	 * saved structure may claim it is already linked even though the
	 * server's collision world currently has no link for it.
	 */
	ent->r.linked = qfalse;

	if (shouldLink &&
		(!(ent->r.svFlags & SVF_CASTAI) || !ent->aiInactive)) {

		trap_LinkEntity(ent);
	}

	/*
	 * Restore BSP areaportal state for movers.
	 *
	 * This mirrors the special handling in RTCW's normal save loader.
	 */
	if (ent->inuse &&
		ent->s.eType == ET_MOVER &&
		ent->moverState != backup.moverState) {

		if (ent->teammaster == ent || !ent->teammaster) {
			if (ent->moverState == MOVER_POS1ROTATE ||
				ent->moverState == MOVER_POS1) {

				/* Restored mover is closed. */
				trap_AdjustAreaPortalState(ent, qfalse);
			}
			else {
				/*
				 * Restored mover is open. Temporarily link it in the
				 * pre-restore state so the portal pair is available,
				 * open the portal, then restore the checkpoint state.
				 */
				backup2 = *ent;

				*ent = backup;
				ent->r.linked = qfalse;
				trap_LinkEntity(ent);
				trap_AdjustAreaPortalState(ent, qtrue);

				*ent = backup2;
				ent->r.linked = qfalse;

				if (shouldLink &&
					(!(ent->r.svFlags & SVF_CASTAI) ||
						!ent->aiInactive)) {

					trap_LinkEntity(ent);
				}
			}
		}
	}

	if (ent->AASblocking) {
		G_SetAASBlockingEntity(ent, qtrue);
	}
}

static void G_RestorePracticeCastState(int index) {
	cast_state_t* cs;
	bot_state_t* bs;

	int character;
	int ms;
	int gs;
	int chatState;
	int ws;

	int numCastScriptEvents;
	cast_script_event_t* castScriptEvents;
	cast_weapon_info_t* weaponInfo;

	cs = &caststates[index];
	bs = cs->bs;

	/*
	 * These are runtime resources. Keep the live objects/handles;
	 * their contents are not part of our raw game-side snapshot.
	 */
	numCastScriptEvents = cs->numCastScriptEvents;
	castScriptEvents = cs->castScriptEvents;
	weaponInfo = cs->weaponInfo;

	*cs = practiceSaveState.castStates[index];

	cs->bs = bs;
	cs->numCastScriptEvents = numCastScriptEvents;
	cs->castScriptEvents = castScriptEvents;
	cs->weaponInfo = weaponInfo;

	if (!practiceSaveState.botStatePresent[index] || !botstates[index]) {
		return;
	}

	bs = botstates[index];

	character = bs->character;
	ms = bs->ms;
	gs = bs->gs;
	chatState = bs->cs;
	ws = bs->ws;

	*bs = practiceSaveState.botStates[index];

	/*
	 * Keep the currently allocated botlib resources.
	 */
	bs->character = character;
	bs->ms = ms;
	bs->gs = gs;
	bs->cs = chatState;
	bs->ws = ws;

	if (bs->gs) {
		trap_BotResetGoalState(bs->gs);
		trap_BotResetAvoidGoals(bs->gs);
	}

	if (bs->ms) {
		trap_BotResetMoveState(bs->ms);
		trap_BotResetAvoidReach(bs->ms);
	}

	if (bs->ws) {
		trap_BotResetWeaponState(bs->ws);
	}

	if (cs->bs && !cs->deathTime) {
		if (cs->entityNum >= 0 &&
			cs->entityNum < MAX_CLIENTS &&
			g_entities[cs->entityNum].client) {

			memset(
				g_entities[cs->entityNum].client->ps.delta_angles,
				0,
				sizeof(g_entities[cs->entityNum].client->ps.delta_angles)
			);

			VectorCopy(cs->ideal_viewangles, cs->viewangles);

			VectorCopy(
				cs->ideal_viewangles,
				g_entities[cs->entityNum].client->ps.viewangles
			);

			memcpy(
				&cs->bs->cur_ps,
				&g_entities[cs->entityNum].client->ps,
				sizeof(playerState_t)
			);

			cs->lastThink = -9999;

			trap_EA_ResetInput(cs->entityNum, NULL);
		}
	}
}