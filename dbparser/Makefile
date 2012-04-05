all: main

main: main.o exec.o err.h commands.o
	g++ main.o exec.o commands.o -Wall -ggdb -o main

commands.o: commands.cpp commands.h
	g++ commands.cpp commands.h -c -Wall -ggdb -std=c++0x 

main.o: main.cpp
	g++ main.cpp -c -Wall -ggdb

exec.o: exec.cpp
	g++ exec.cpp -c -Wall -ggdb

clean:
	rm *.o main
