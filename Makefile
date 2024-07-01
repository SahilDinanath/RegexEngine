

all: regex_engine.c
	gcc -c regex_engine.c -o regex_engine.o

clean:
	rm regex_engine.o
