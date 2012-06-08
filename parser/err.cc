#include <cerrno>
#include <cstdio>
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

const char* SystemError::what() {
	return what_.c_str();
}

void syserr(const char* message) throw (SystemError) {
	fprintf(stderr, "%s\nERRNO: %d\n", message, errno);
	throw SystemError(std::string(message));
}

