#ifndef SPEEDRUN_H
#define SPEEDRUN_H

#define SPEEDRUN_STATE_MAGIC		0x52544357u
#define SPEEDRUN_STATE_VERSION		1u

#define SR_STATE_LEVEL_TRANSITION	( 1u << 0 )
#define SR_STATE_LOADING			( 1u << 1 )
#define SR_STATE_CUTSCENE			( 1u << 2 )
#define SR_STATE_PLAYER_FROZEN		( 1u << 3 )
#define SR_STATE_CONTROL_LOCKED		( 1u << 4 )

typedef struct {
    unsigned int magic;
    unsigned int version;
    unsigned int flags;

    unsigned int mapSequence;
    unsigned int transitionSequence;

    char mapName[32];
} speedrunState_t;

#endif