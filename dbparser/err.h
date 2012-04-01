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
extern void syserr() throw (SystemError) {
	throw SystemError();
}

#endif
