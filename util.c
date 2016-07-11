#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "util.h"

struct _Movie
{
	char name[MAX_LINE_LEN];
	char filename[MAX_LINE_LEN];
	char path[2*MAX_LINE_LEN];
	char valid; // used as gboolean
};

struct _Movie movies[MAX_MOVIES];

char *long2string(long number)
{
	static char string[22];
	sprintf(string, "%li", number);
	return string;
}

long string2long(char *string)
{
	return strtol(string, NULL, 10);
}

FILE *open_file(char *filename, long long *filesize)
{
	struct stat filestats;
	stat(filename, &filestats);
	*filesize = filestats.st_size;

	static FILE *stream;
	stream = fopen(filename, "rb");

	if (!stream)
		fprintf(stderr, "ERROR: Video File not found!\n");

	return stream;
}

char *getl(FILE *stream, char *line) // without \n at the end
{
	char *ret;
	ret = fgets(line, MAX_LINE_LEN, stream);
	line[strlen(line)-1] = 0x00;

	return ret;
}

char readmovies()
{
	int mnum;
	for (mnum = 0; mnum < MAX_MOVIES; mnum++)
		movies[mnum].valid = FALSE;

	FILE *stream;
	stream = fopen(CONFIG_FILE, "rb");

	if (!stream) return FALSE;

	char line[MAX_LINE_LEN];
	char basedir[MAX_LINE_LEN];
	char basedir_string[MAX_LINE_LEN];


	// Parse config file
	mnum = 0;
	while(getl(stream, line))
	{
		if (strings_equal(line, "__BASEDIR"))
		{
			assert(getl(stream, basedir_string));
			if (!strstr(basedir, "__CWD__"))
			{
				strcpy(basedir, basedir_string);
			}
		}

		if (strings_equal(line, "__MOVIE"))
		{
			assert(getl(stream, movies[mnum].name));
			assert(getl(stream, movies[mnum].filename));

			movies[mnum].valid = TRUE;
			++mnum;
		}
	}

	// Establish Paths to Files
	mnum = 0;
	while(movies[mnum].valid)
	{
		strcpy(movies[mnum].path, basedir);
		strcat(movies[mnum].path, movies[mnum].filename);
		++mnum;
	}

	// Dump Paths
	mnum = 0;
	while(movies[mnum].valid) fprintf(stderr, "Adding File: %s\n", movies[mnum++].path);

	fclose(stream);

	return TRUE;
}

// Helper for on_movlist_selection_changed to retrieve the filename / path from the list name
char *movie_get_path_by_name(char *name)
{
	int mnum = 0;
	while(movies[mnum].valid)
	{
		if (strings_equal(movies[mnum].name, name))
			return movies[mnum].path;
		++mnum;
	}

	return NULL;
}

char **get_movie_names()
{
	static char *movienames[MAX_LINE_LEN];

	int mnum = 0;
	while(movies[mnum].valid)
	{
		movienames[mnum] = movies[mnum].name;
		++mnum;
	}

	return movienames;
}
