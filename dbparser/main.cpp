#include "exec.h"
#include <iostream>
#include <vector>
#include <string>

using std::string;
using std::vector;

int main() {

	vector<string> arguments;
	arguments.push_back("make");
	arguments.push_back("-n");
	arguments.push_back("-o");
	arguments.push_back("msp.o");

	//char * a[] = {"make", "-n", "-o", "msp.o", (char*) 0};

	std::cout << exec("make", arguments) << std::endl;
	return 0;

}

