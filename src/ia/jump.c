#include "ia.h"
#include "ia_utils.h"
#include "board.h"
#include "move.h"

char* name = "Jump";

// Move to the closest vertex to the finish
size_t move_forward(struct game_state_t game) {

	enum direction_t GOAL = game.self.color == BLACK ? SOUTH : NORTH;
    size_t pos = GOAL == SOUTH ? game.self.pos + 2*sqrt(game.graph->num_vertices) : game.self.pos - 2*sqrt(game.graph->num_vertices) ;

    return pos;
}

struct move_t make_first_move(struct game_state_t game) {
	return make_default_first_move(game);
}

// Jerry's strategy is to run directly to the arrival without placing any walls
struct move_t make_move(struct game_state_t game) {
	struct move_t move;
	move.c = game.self.color;
	move.t = MOVE;
	move.m = move_forward(game);
	move.e[0] = no_edge();
	move.e[1] = no_edge();

	return move;
}

void finalize_ia() {
	// do nothing
}
