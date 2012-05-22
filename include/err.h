#ifndef ERR_ERR_H_
#define ERR_ERR_H_

#include <exception>
#include <errno.h>
#include <string.h>

/**
 * An exception thrown when errno indicates system error.
 */
class SystemError : public std::exception {
	public:
		virtual const char* what() const throw() {
			return strerror(errno);
		}
};

/* Throws SystemError and prints message and errno to stderr.
 */
extern void syserr(const char* message) throw (SystemError);

#endif	// ERR_ERR_H_
