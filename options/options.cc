#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include "err.h"
#include "options.h"



options prepare_options(int argc, char **argv){
	std::vector<std::string> forward;
	std::vector<std::string>targets;
	char version[] = "yadmake 0.1 ";

	bool exec = 0;
	bool is_p = 0;
	int is_clean = 0;

	int c;
	const char *opt = "bmBdeikLno:pqrRsStvW:";
	std::string temp;
	options result;
	result.keep_going = 0;
	result.dist_make = 0;
	result.exec = "";

	while ((c = getopt (argc, argv, opt)) != -1) {
		switch (c) {
			case 'p':
				is_p = 1;
				break;
			case 'v':
				std::cout << version << std::endl;
				return result;
			case 'n':
			case 'q':
			case 't':
				exec = 1;
				break;
			case 'k':
				result.keep_going = 1;
			case 'b':
			case 'm':
			case 'B':
			case 'd':
			case 'e':
			case 'i':
			case 'L':
			case 'r':
			case 'R':
			case 's':
			case 'S':
				temp = argv[optind - 1];
				forward.push_back(temp);
				break;
			case 'o':
			case 'W':
				temp = argv[optind - 1];
				forward.push_back(temp);
				temp = optarg;
				forward.push_back(temp);
				break;
			case '?':
				if (optopt == 'o' || optopt == 'W')
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr,
							"Unknown option character `\\x%x'.\n",
							optopt);
				syserr("");
				return result;

			default:
				break;
		}
	};

	int i = 0;

	for (i = optind; i < argc; i++) {
		temp = argv[i];
		if (temp.compare("clean") == 0) {
			is_clean = i;
		}
	};

	temp = "";

	if (exec) {
		temp = "make ";
		for (i = 1; i < argc; i++) 
			temp = temp + argv[i] + " ";
		result.dist_make = 0;
	}
	else {
		if (is_clean != 0) {
			temp = "make clean ";
			for (i = 0; i < (int) forward.size(); i++) 
				temp = temp + forward[i] + " ";
		}
		if (is_p) {
			if (!temp.empty())
				temp += "-p -f /dev/null ";
			else 
				temp = "make -p -f /dev/null ";
		}

		if (is_clean != argc - 1 || argc == 1) {
			result.dist_make =1;
			int j;
			if (is_clean == 0)
				j = optind;
			else j = is_clean + 1;
			for (i = j;  i < argc; i++) {
				targets.push_back(argv[i]);
			}
		}
		else
			result.dist_make = 0;
	}

	result.forward = forward;
	result.exec = temp;
	result.targets = targets;

	return result;
}
