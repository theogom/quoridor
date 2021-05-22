/**
 * @file crashboy.c
 *
 * @brief A guinea pig used to realize tests
 */

#include "ia.h"
#include "ia_utils.h"
#include "board.h"
#include "move.h"

char* name = "Dummy";

struct move_t make_first_move(struct game_state_t game) {
	return make_default_first_move(game);
}

struct move_t make_move(struct game_state_t game) {
    return (struct move_t) {
    	.m = no_vertex(),
    	.e = {no_edge(), no_edge()},
    	.t = NO_TYPE,
    	.c = game.self.color
    };
}

void finalize_ia() {
	// do nothing
}
