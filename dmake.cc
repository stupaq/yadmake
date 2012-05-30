
#include <iostream>

#include "dbparser.h"
#include "worker.h"
#include "dispatcher.h"

int main()
{
	Messaging m;
	std::vector<Worker*> workers = get_workers(&m);
	return 0;
}
