all: main

main: main.o exec.o err.o commands.o
	g++ main.o exec.o err.o commands.o -Wall -ggdb -o main

commands.o: commands.cpp
	g++ commands.cpp -c -Wall -ggdb -std=c++0x 

main.o: main.cpp
	g++ main.cpp -c -Wall -ggdb

exec.o: exec.cpp
	g++ exec.cpp -c -Wall -ggdb

err.o: err.cpp
	g++ err.cpp -c -Wall -ggdb

clean:
	rm *.o main
