ALL =  dispatcher

CC = g++
LDFLAGS = -Wall --pedantic

all: $(ALL)

% : %.c
	$(CC) $(LDFLAGS) $<

clean:
	rm $(ALL) *~ *.o
