#ifndef _EXEC_
#define _EXEC_

#include <string>
#include <vector>

// Executes programme with args as arguments
// and returns stdout of this programme as a string.
extern std::string exec(const std::string& programme,
		const std::vector<std::string>& arguments);

#endif
