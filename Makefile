flags=-O2 -Wall -std=c2x -I.
ldflags=

.PHONY: clean

all: myhtml

myhtml: utils.o myhtml.o lexer.o parser.o
	cc $(flags) $^ -o $@ $(ldflags)

myhtml.o: myhtml.c myhtml.h
	cc $(flags) -c $<

lexer.o: lexer.c lexer.h
	cc $(flags) -c $<

parser.o: parser.c parser.h
	cc $(flags) -c $<

utils.o: utils.c utils.h
	cc $(flags) -c $<

clean:
	rm -f *.o myhtml