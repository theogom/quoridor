/**
 * @file player_test.c
 *
 * @brief Contains the tests on player.c 
 */ 



#include "tests.h"
#include "player.h"
#include "ia.h"
#include "move.h"
#include "board.h"
#include "opt.h"
#include "math.h"
#include <stdio.h>
#include <string.h>

extern struct game_state_t game;
extern char *name;

size_t n = 3;
size_t edges = 0;
struct graph_t *board = NULL;

static void setup(void) {
	n = 3;
	board = graph_init(n, SQUARE);
	edges = 2 * n * (n - 1);
	initialize(BLACK, board, ceil(edges / 15.0));
}

static void teardown(void) {
	finalize();
}

void test_initialization(void) {
	printf("%s", __func__);

	if (game.self.color != BLACK) {
		FAIL("Id initialization failed");
	}

	if (game.self.num_walls != ceil(edges / 15.0)) {
		FAIL("Wrong number of walls initialized");
	}

	// TODO Check game.graph
}

void test_get_player_name(void) {
	printf("%s", __func__);

	if (strcmp(get_player_name(), name) != 0) {
		FAIL("Can not get game name");
	}
}

void test_play_random(void) {
	printf("%s", __func__);
}

void test_player_main(void) {
	TEST(test_initialization);
	TEST(test_get_player_name);
	// TEST(test_play_random);

	SUMMARY();
}
