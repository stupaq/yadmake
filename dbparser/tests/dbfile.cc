#include <cstdio>

#include "../dbparser.h"

int main(int argc, char *argv[]) {

	if (argc != 2)
		return 1;

	FILE* fp = fopen(argv[1], "r");

	if (fp) {
		int pipe = fileno(fp);
		DependencyGraph dg(pipe);
	} else
		return 1;

	pclose(fp);
	return 0;
}



