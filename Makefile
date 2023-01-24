CC = /usr/bin/gcc
# CFLAGS = -Wall -g -O2 -Werror -std=gnu99 -Wno-unused-function
CFLAGS = -Werror -g 

SRC = .

HCC = a.out

# SRC
FILES = $(wildcard $(SRC)/src/*.c)
# exclude main.c
OUT_FILE = $(SRC)/src/main.c
SRC_FILES = $(filter-out $(OUT_FILE), $(FILES))

run: main
	./$(HCC)

main:$(FILES)
	$(CC) $(FILES) -I ./src $(CFLAGS) -o $(HCC)


# test
TEST = $(wildcard $(SRC)/test/*.c)
TARGETS = $(patsubst %.c, %, $(TEST))  

test:$(TARGETS) 
	$(TARGETS)

$(TARGETS):%:%.c $(SRC_FILES) 
	$(CC) $< $(SRC_FILES) -I ./src $(CFLAGS) -o $@ 

.PHONY:run main clean test

clean:
	rm -f $(HCC) $(TARGETS)