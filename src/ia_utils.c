#include "ia_utils.h"

struct move_t make_default_first_move(struct game_state_t game) {
	size_t vertex_owned = 0;
	for (size_t i = 0; i < game.graph->num_vertices; i++)
		if (gsl_spmatrix_uint_get(game.graph->o, 0, i) == 1)
			vertex_owned++;

	size_t first_v = gsl_spmatrix_uint_get(game.graph->o, game.self.color, 0) ? (size_t)vertex_owned/2 : game.graph->num_vertices - (size_t)(vertex_owned/2);

	if (gsl_spmatrix_uint_get(game.graph->o, game.self.color, first_v) == 0){//If by mistake the chosen vertex don't belong to the player
		for (size_t i = 0; i < game.graph->num_vertices; i++)
			if (gsl_spmatrix_uint_get(game.graph->o, game.self.color, i)){
				first_v = i;
				break;
			}
	}
	struct move_t move = {
			.e = {no_edge(), no_edge()},
			.t = MOVE,
			.c = game.self.color,
			.m = first_v
	};

	return move;
}
