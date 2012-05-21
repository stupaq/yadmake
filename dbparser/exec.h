#ifndef EXEC_EXEC_H_
#define EXEC_EXEC_H_

#include <string>
#include <vector>
#include <utility>

/* Executes programme with args as arguments
 * @and returns a pair consisting of stdout and stderr
 * @of this programme as strings.
 */
extern std::pair<std::string, std::string> exec(const std::string& programme,
		const std::vector<std::string>& arguments);

#endif	// EXEC_EXEC_H_
