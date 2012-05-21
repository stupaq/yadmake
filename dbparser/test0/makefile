ALL = main

all: $(ALL)

main: main.o a.o
	g++ $^ -o $@

%.o: %.cpp a.h
	g++ $< -c -o $@

clean:
	rm *~ *.o *.d $(ALL) 

blah:
	@echo blah
