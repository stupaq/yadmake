#ifndef _ERR_
#define _ERR_

// Prints error_message and errno to stderr
// and exits programme with code 1.
// TODO: may be we prefer exceptions?
extern void syserr(const char* error_message);

#endif
