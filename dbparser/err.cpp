#include "err.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void syserr(const char* error_message) {
	fprintf(stderr, "ERROR: %s\n", error_message);
	fprintf(stderr, "%d %s\n", errno, strerror(errno));
	exit(1);
}
