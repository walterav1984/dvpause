#include <stdio.h>

#ifndef _DV_HEADER
#define _DV_HEADER

enum {
	PLAYER_STATE_CONTINUE,
	PLAYER_STATE_NEWSOURCE, // practically not used, but available
	PLAYER_STATE_PAUSE,
	PLAYER_STATE_PAUSE_NEWSOURCE,
	PLAYER_STATE_NEWPOS,
	PLAYER_STATE_NEWPOS_PERCENT,
	PLAYER_STATE_TERMINATE,
	PLAYER_STATE_TERMINATED
} playerstate;

void *putdv(void *asdf);
void init_dvsystem();
long get_total_frames(long long filesize);
void putframe_noaudio(FILE *stream);
void putframe(FILE *stream);

#endif
