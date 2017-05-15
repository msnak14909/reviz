CC=gcc

ifeq ($(DEBUG),1)
CFLAGS=-fPIC -std=c11 -O2 -ggdb -march=native -Wall -Wextra -Wshadow -fno-omit-frame-pointer -fsanitize=address
else
CFLAGS=-fPIC -std=c11 -O2 -ggdb -march=native -Wall -Wextra -Wshadow
endif
CFLAGS+= -Iinclude

HEADERS=./include/r.h

C_FILES = $(filter-out src/main.c, $(wildcard src/*.c))
OBJECTS=$(notdir $(C_FILES:.c=.o))

all: reviz $(OBJECTS)

%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

reviz: $(OBJECTS) ./src/main.c $(HEADERS)
	$(CC) $(CFLAGS) ./src/main.c $(OBJECTS) -o $@
	@echo "#! /bin/bash" >> $@_dot
	@echo '$(PWD)/reviz "$$@"' >> reviz_dot
	@echo 'dot -Tpng nfa_out.dot -o nfa.png' >> reviz_dot
	@echo 'dot -Tpng dfa_out.dot -o dfa.png' >> reviz_dot
	@echo 'dot -Tpng opt_out.dot -o opt.png' >> reviz_dot
	@chmod +x reviz_dot

.PHONY: clean
clean:
	-rm $(OBJECTS) reviz reviz_dot *.png *.dot
