flags=-O2 -Wall -std=c2x
ldflags=

.PHONY: all clean

all: clean myhtml

myhtml: utils.o myhtml.o
	cc $(flags) $^ -o $@ $(ldflags)

myhtml.o: myhtml.c myhtml.h
	cc $(flags) -c $<

utils.o: utils.c utils.h
	cc $(flags) -c $<

clean:
	rm -f *.o myhtml