#include <stdio.h>
#include "config.h"

#ifndef _UTIL_H
#define _UTIL_H

#define strings_equal(S1,S2) ((!strcmp(S1,S2)))

char *long2string(long number);
long string2long(char *string);
FILE *open_file(char *filename, long long *filesize);
char *getl(FILE *stream, char *line); // without \n at the end
char readmovies();
char *movie_get_path_by_name(char *name);
char **get_movie_names();

#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif
