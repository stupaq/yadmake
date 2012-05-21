#include <exception>
#include <errno.h>
#include <string.h>
#include <cstdio>
#include "err.h"

void syserr(const char* message) throw (SystemError) {
	fprintf(stderr, "%s\nERRNO: %d\n", message, errno);
	throw SystemError();
}

