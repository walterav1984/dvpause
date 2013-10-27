#include "config.h"
#include "util.h"

#ifndef _GLOBAL_H
#define _GLOBAL_H

extern struct _DVControl {
	// File Controls
	char *filename;

	// Position Controls
	off_t newpos;
	double pos_percent;

	long totalframes;
	long currentframe;
} DVControl;

#endif
