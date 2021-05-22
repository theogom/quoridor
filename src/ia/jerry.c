#include "ia.h"
#include "ia_utils.h"
#include "board.h"
#include "move.h"

char* name = "Jerry";

// Move to the closest vertex to the finish
size_t move_forward(struct game_state_t game) {
	size_t linked[MAX_DIRECTION];
	size_t num = get_linked(game.graph, game.self.pos, linked);

	if (num == 0)
		fprintf(stderr, "ERROR: Player is blocked\n");

	// TODO get the closest to the finish
	enum direction_t GOAL = game.self.color == BLACK ? SOUTH : NORTH;

	enum direction_t directions[] = { GOAL, WEST, EAST, linked[opposite(GOAL)] };

	for (int i = 0; i < 4; ++i) {
		if (!is_no_vertex(linked[directions[i]]) && linked[directions[i]] != game.opponent.pos) {
			return linked[directions[i]];
		}
	}

	return no_vertex();
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
