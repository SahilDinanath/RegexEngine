CODEDIRS=.
INCDIRS=.

OPT=-O0
CC= gcc
CFLAGS=-Wall -Wextra -g $(foreach D,$(INCDIRS), -I$(D)) 

CFILES=$(foreach D, $(CODEDIRS), $(wildcard $(D)/*.c))
OBJECTS=$(patsubst %.c, %.o, $(CFILES))

all: $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf $(OBJECTS) $(BIN)
