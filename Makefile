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

# run: main
# 	./$(HCC)

main:$(FILES)
	$(CC) $(FILES) -I ./src $(CFLAGS) -o $(HCC)

# 拿到test目录下的所有文件
TEST = $(wildcard $(SRC)/test/*.c)
# 去除所有文件的后缀名字
TARGETS = $(patsubst %.c, %, $(TEST))  

$(TARGETS):%:%.c $(SRC_FILES) 
	@$(CC) $< $(SRC_FILES) -I ./src $(CFLAGS) -o $@

test:$(TARGETS) 
	@sh -c 'for file in $(TARGETS);do $$file ;done;'

.PHONY:run main clean test

clean:
	rm -f $(HCC) $(TARGETS)