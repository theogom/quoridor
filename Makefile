GSL_PATH ?= gsl
CFLAGS = --std=c99 -Wall -Wextra -O3 -Iheaders -I$(GSL_PATH)/include
LFLAGS = -L$(GSL_PATH)/lib -lgsl -lgslcblas -ldl -lm
CC = gcc

.PHONY: build test run_server run_tests install clean

all: build

# SCRIPTS

run-server: build/server
	LD_LIBRARY_PATH=$(GSL_PATH)/lib ./build/server $(ARGS)

run-tests: build/alltests
	LD_LIBRARY_PATH=$(GSL_PATH)/lib ./build/alltests

run-game: build/server build/geralt.OLD.so build/geralt.so
	LD_LIBRARY_PATH=$(GSL_PATH)/lib ./build/server ./build/geralt.OLD.so ./build/geralt.so -m 5

test: build/alltests
	LD_LIBRARY_PATH=$(GSL_PATH)/lib ./build/alltests

install: build/server build/alltests build/pablo_supersaiyan.so build/geralt.so
	cp $^ install

clean:
	find build install -type f -not -name .keep | xargs rm -f

build: build/server build/pablo.so build/pablo_supersaiyan.so build/geralt.so build/goodboy.so

build/server: build/main.o build/server.o build/opt.o build/board.o
	$(CC) $^ -o $@ $(LFLAGS)

build/alltests: build/tests.o build/player_test.o build/server_test.o build/crashboy.so build/ia_utils.o build/player.o build/board.o build/opt.o  build/server.o
	$(CC) $^ -o $@ --coverage $(LFLAGS)

# OBJ

build/%.o: src/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

build/%.o: src/ia/%.c
	$(CC) -c -fPIC $< -o $@ $(CFLAGS)

build/player.o: src/player.c
	$(CC) -c -fPIC $< -o $@ $(CFLAGS)

# TSTOBJ

build/%.o: tests/%.c
	$(CC) -c $< -o $@ --coverage $(CFLAGS)

build/crashboy.o: tests/crashboy.c
	$(CC) -c -fPIC $< -o $@ $(CFLAGS)

build/jump.o: src/ia/jump.c
	$(CC) -c -fPIC $< -o $@ $(CFLAGS)

# DYNAMIC LIBS

build/%.so: build/%.o build/player.o build/board.o build/ia_utils.o 
	$(CC) -shared $^ -o $@ $(LFLAGS)

build/jump.so: build/jump.o build/player.o build/board.o build/ia_utils.o
	$(CC) -shared $^ -o $@ $(LFLAGS)
