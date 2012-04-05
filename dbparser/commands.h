#include "dbparser.hpp"
#include <vector>
#include <string>

/* Adds commands to targets */
void count_commands(DependencyGraph* graph,
		const std::vector<std::string>& basics,
		const std::string& delimiter);
