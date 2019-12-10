
DEBUG=-g3
OPT=-O0
WARN=-pedantic -Werror -Wextra
CFLAGS=-std=gnu18 $(WARN) $(OPT) $(DEBUG)

OBJS=array.o bigint.o main.o ops.o
HDRS=array.h bigint.h math.h

.PHONY: all clean run

all: main

$(OBJS): $(HDRS)

main: $(OBJS)
	gcc $^ -o $@

run: main
	./$^

clean:
	rm -f $(OBJS) main

