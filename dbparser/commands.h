#ifndef COMMANDS_COMMANDS_H_
#define COMMANDS_COMMANDS_H_

#include "dbparser.h"
#include <vector>
#include <string>

/* Adds commands to targets.
 * @basic can contain for example 'make', 'aim', '-o', 'ready_aim'.
 * @delimiter seems to be blah always
 */
void count_commands(DependencyGraph* graph,
		const std::vector<std::string>& basics,
		const std::string& delimiter);

#endif	// COMMANDS_COMMANDS_H_
