#include <cerrno>
#include <cstring>

#include <exception>
#include <string>

#include "err.h"

using namespace std;

SystemError::SystemError(const string& what) : errno_(errno) {
	if (what.empty())
		what_ = string(strerror(errno_));
	else
		what_ = what;
}

SystemError::~SystemError() throw () {
}

const char* SystemError::what() throw () {
	return what_.c_str();
}

void syserr(const char* message) throw (SystemError) {
	throw SystemError(std::string(message));
}

