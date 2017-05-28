#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "global.h"
#include "dv.h"
#include "util.h"


/*
	Start putdv thread
*/
void init_dvsystem()
{
	pthread_t printthread;
	int rc = pthread_create(&printthread, NULL, putdv, NULL);
	assert(rc == 0);
}


/*
	Thread that outputs all DV data
*/
void *putdv(void *asdf)
{
	// Init - Wait until main program is ready
	fpos_t position;
	long long filesize = 0;

	// In order to start streaming, filename must be given and playerstate must contain NEWSOURCE
	// If program is supposed to be terminated, also continue
	while((!DVControl.filename
		|| (playerstate != PLAYER_STATE_NEWSOURCE
		&&  playerstate != PLAYER_STATE_PAUSE_NEWSOURCE))
		&&  playerstate != PLAYER_STATE_TERMINATE)
		usleep(10);

	FILE *stream = NULL;

	for(;;)
	{
		if (playerstate == PLAYER_STATE_CONTINUE)
		{
			DVControl.currentframe++;
			putframe(stream);
			if (stream) fgetpos(stream, &position);
		}
		else if (playerstate == PLAYER_STATE_PAUSE)
		{
			putframe_noaudio(stream);
			if (stream) fsetpos(stream, &position);
		}
		else if (playerstate == PLAYER_STATE_NEWSOURCE)
		{
			if (stream) fclose(stream);
			stream = open_file(DVControl.filename, &filesize);
			DVControl.totalframes = get_total_frames(filesize);
			playerstate = PLAYER_STATE_CONTINUE;
			DVControl.currentframe = 0;
			DVControl.pos_percent = 0;
		}
		else if (playerstate == PLAYER_STATE_PAUSE_NEWSOURCE)
		{
			if (stream) fclose(stream);
			stream = open_file(DVControl.filename, &filesize);
			DVControl.totalframes = get_total_frames(filesize);
			if (stream) fgetpos(stream, &position);
			playerstate = PLAYER_STATE_PAUSE;
			DVControl.currentframe = 0;
			DVControl.pos_percent = 0;
		}
		else if (playerstate == PLAYER_STATE_NEWPOS)
		{
			if (DVControl.newpos < filesize && DVControl.newpos >= 0)
			{
				if (stream) fseeko(stream, DVControl.newpos, SEEK_SET);
				DVControl.currentframe = DVControl.newpos / BYTES_PER_FRAME;
				if (stream) fgetpos(stream, &position);
			}

			playerstate = PLAYER_STATE_PAUSE;
		}
		else if (playerstate == PLAYER_STATE_NEWPOS_PERCENT)
		{
			long newframe = DVControl.pos_percent * DVControl.totalframes / 100;
			DVControl.newpos = newframe * BYTES_PER_FRAME;

			playerstate = PLAYER_STATE_NEWPOS;
		}
		else if (playerstate == PLAYER_STATE_TERMINATE)
		{
			break;
		}

		if (DVControl.currentframe >= (DVControl.totalframes -1)
		&& playerstate == PLAYER_STATE_CONTINUE)
			playerstate = PLAYER_STATE_PAUSE;
	}

	if (stream) fclose(stream);

	playerstate = PLAYER_STATE_TERMINATED;
	pthread_exit(NULL);
}


/*
	Retrieve total amount of frames in a file from the filesize
*/
long get_total_frames(long long filesize)
{
	return filesize / (80 * 150 * SEQUENCES);
}


/*
	Put frame to stdout and remove audio, used for pausing
*/
// DV Format Information: http://dvswitch.alioth.debian.org/wiki/DV_format/
void putframe_noaudio(FILE *stream)
{
	if (!stream) return;

	int block, sequence, pack; // make up 1 frame

	for (pack = 0; pack<SEQUENCES; ++pack)
	{
		for (sequence = 0; sequence < 150; ++sequence)
		{
			if (((sequence-6) % 16) != 0) // no Audio source (AS)
			{
				for (block = 0; block < 80; ++block)
					printf("%c", getc(stream));
			}
			else // Audio block
			{
				for (block = 0; block < 8; ++block)
					printf("%c", getc(stream));
				for (block = 0; block < 72; ++block)
				{
					getc(stream);
					printf("%c", 0x00);
				}
			}
		}
	}
}


/*
	Put frame to stdout, with audio
*/
void putframe(FILE *stream)
{
	if (!stream) return;

	int i;
	for (i=0; i<BYTES_PER_FRAME; ++i)
		printf("%c", getc(stream));
}
