#ifndef COMMANDS_COMMANDS_H_
#define COMMANDS_COMMANDS_H_

#include <vector>
#include <string>

#include "dbparser.h"

/**
 * An exception thrown by count_commands if make -n
 * @exits with error.
 */
class MakeError : public std::runtime_error {
	public:
		MakeError(const std::string& what): std::runtime_error(what) {}
};

/**
 * Adds commands to targets.
 * @basic can contain for example 'make', 'aim', '-o', 'ready_aim'.
 * @delimiter seems to be blah always
 */
void count_commands(DependencyGraph* graph,
		const std::vector<std::string>& basics,
		const std::string& delimiter);

#endif	// COMMANDS_COMMANDS_H_
