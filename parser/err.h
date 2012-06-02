#ifndef _ERR_
#define _ERR_

#include <exception>
#include <errno.h>
#include <string.h>

class SystemError : public std::exception {
	public:
		virtual const char* what() const throw() {
			return strerror(errno);
		}
};

// Throws SystemError.
extern void syserr(const char* message) throw (SystemError) {
	fprintf(stderr, "%s\nERRNO: %d\n", message, errno);
	throw SystemError();
}

#endif
