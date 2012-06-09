#ifndef _ERR_H_
#define _ERR_H_

#include <cerrno>

#include <exception>
#include <string>

/**
 * An exception thrown when errno indicates system error.
 */
class SystemError : public std::exception {
	private:
		const int errno_;
		std::string what_;
	public:
		SystemError(const std::string& what = "");
		~SystemError() throw ();
		virtual const char* what() throw ();
};

/*
 * Throws SystemError and prints message and errno to stderr.
 */
extern void syserr(const char* message) throw (SystemError);

#endif	// ERR_ERR_H_
