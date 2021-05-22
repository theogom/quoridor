#define _DEFAULT_SOURCE

#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "ia.h"
#include "move.h"

#define EDGE(graph, i, j) ((graph)[(i) * n2 + (j)])

#define DISPLACEMENT_MOVE(inc)        (MOVE << 24 | ((inc) & 0xFFFF))
#define WALL_MOVE(corner, horizontal) (WALL << 24 | (horizontal) << 16 | ((corner) & 0xFFFF))

#define QUEUE_ADD(array, capacity, start, size, value) ((array)[((start) + (size)++) % (capacity)] = (value))
#define QUEUE_REMOVE(array, capacity, start, size) ((array)[(start)++ % (capacity)]); --(size)

#define SCORE_LIMIT INT_MAX
#define INVALID_MOVE_SCORE (-SCORE_LIMIT)
#define WIN_SCORE (SCORE_LIMIT)
#define LOOSE_SCORE (-WIN_SCORE)

#define TOTAL_TIME_AVAILABLE 15000
#define AVG_NB_OF_TURN 100

// define simplified structures to gain efficiency

typedef struct {
	char *graph;
	int pos;
	int num_walls;
	int opponent_pos;
	int opponent_num_walls;
	int *start_pos;
	int *opponent_start_pos;
	bool target_is_up;
} SimpleGameState;


// data initialized once (doesn't change from one move to another)

char *name = "Geralt";
int n;
int n2;
int n4;
int nb_of_start_pos;
int *start_pos;
int *opponent_start_pos;
bool target_is_up;

void add_displacement_moves(const char *graph, unsigned *moves, int *nb_of_moves, int player_pos, int opponent_pos, char main_direction, char secondary_direction) {
	char edge;

	int step_1 = player_pos + main_direction;
	if (step_1 < 0 || step_1 >= n2 || (edge = EDGE(graph, player_pos, step_1)) == 0 || edge > 4) {
		return;
	}

	if (step_1 != opponent_pos) {
		moves[(*nb_of_moves)++] = DISPLACEMENT_MOVE(main_direction);
		return;
	}

	int step_2 = step_1 + main_direction;
	if (step_2 >= 0 && step_2 < n2 && (edge = EDGE(graph, step_1, step_2)) >= 1 && edge <= 4) {
		moves[(*nb_of_moves)++] = DISPLACEMENT_MOVE(step_2 - player_pos);
		return;
	}

	step_2 = step_1 + secondary_direction;
	if (step_2 >= 0 && step_2 < n2 && (edge = EDGE(graph, step_1, step_2)) >= 1 && edge <= 4) {
		moves[(*nb_of_moves)++] = DISPLACEMENT_MOVE(step_2 - player_pos);
	}

	step_2 = step_1 - secondary_direction;
	if (step_2 >= 0 && step_2 < n2 && (edge = EDGE(graph, step_1, step_2)) >= 1 && edge <= 4) {
		moves[(*nb_of_moves)++] = DISPLACEMENT_MOVE(step_2 - player_pos);
	}
}

void add_wall_moves(const char *graph, unsigned *moves, int *nb_of_moves) {
	for (int i = 0; i < n - 1; ++i) {
		for (int j = 0; j < n - 1; ++j) {
			/*
			 * a -e- b
			 * |     |
			 * g     h
			 * |     |
			 * c -f- d
			 */
			int a = i * n + j;
			int b = a + 1;
			int c = a + n;
			int d = b + n;
			char e = EDGE(graph, a, b);
			char f = EDGE(graph, c, d);
			char g = EDGE(graph, a, c);
			char h = EDGE(graph, b, d);

			if (e >= 1 && e <= 4 && f >= 1 && f <= 4 && g != 7) {
				moves[(*nb_of_moves)++] = WALL_MOVE(a, false);
			}

			if (g >= 1 && g <= 4 && h >= 1 && h <= 4 && e != 5) {
				moves[(*nb_of_moves)++] = WALL_MOVE(a, true);
			}
		}
	}
}

unsigned *get_possible_moves(SimpleGameState *game, int *nb_of_moves) {
	unsigned *moves;

	if (game->pos == -1) {
		moves = malloc(nb_of_start_pos * sizeof(unsigned));
		for (int i = 0; i < nb_of_start_pos; ++i) {
			moves[i] = DISPLACEMENT_MOVE(game->start_pos[i] - game->pos);
		}
		*nb_of_moves = nb_of_start_pos;
		return moves;
	}

	moves = malloc((2 * (n - 1) * (n - 1) + 5) * sizeof(unsigned));
	*nb_of_moves = 0;

	add_displacement_moves(game->graph, moves, nb_of_moves, game->pos, game->opponent_pos, 1, (char) n);
	add_displacement_moves(game->graph, moves, nb_of_moves, game->pos, game->opponent_pos, -1, (char) n);
	add_displacement_moves(game->graph, moves, nb_of_moves, game->pos, game->opponent_pos, (char) n, 1);
	add_displacement_moves(game->graph, moves, nb_of_moves, game->pos, game->opponent_pos, (char) -n, 1);

	if (game->num_walls > 0) {
		add_wall_moves(game->graph, moves, nb_of_moves);
	}

	moves = realloc(moves, *nb_of_moves * sizeof(unsigned));
	return moves;
}

int distance(SimpleGameState *game, int pos, bool self) {
	bool current_target_is_up = self == game->target_is_up;
	int dist;

	bool flags[n2];
	memset(flags, true, n2 * sizeof(bool));
	flags[pos] = false;

	int queue[n2];
	int queue_start = 0;
	int queue_size;

	int iteration_length;
	int next_iteration_length = 0;

	if (pos == -1) {
		dist = 1;
		iteration_length = nb_of_start_pos;
		queue_size = nb_of_start_pos;

		for (int i = 0; i < nb_of_start_pos; ++i) {
			queue[i] = (self ? game->start_pos : game->opponent_start_pos)[i];
		}

	} else {
		dist = 0;
		iteration_length = 1;
		queue_size = 1;
		queue[0] = pos;
	}

	while (queue_size > 0) {
		int current_pos = QUEUE_REMOVE(queue, n2, queue_start, queue_size);

		if ((current_target_is_up && current_pos < n) || (!current_target_is_up && current_pos >= n2 - n)) {
			return dist;
		}

		int new_pos;
		char edge;

		// north
		new_pos = current_pos - n;
		if (new_pos >= 0 && flags[new_pos] && (edge = EDGE(game->graph, current_pos, new_pos)) >= 1 && edge <= 4) {
			flags[new_pos] = false;
			QUEUE_ADD(queue, n2, queue_start, queue_size, new_pos);
			++next_iteration_length;
		}

		// south
		new_pos = current_pos + n;
		if (new_pos < n2 && flags[new_pos] && (edge = EDGE(game->graph, current_pos, new_pos)) >= 1 && edge <= 4) {
			flags[new_pos] = false;
			QUEUE_ADD(queue, n2, queue_start, queue_size, new_pos);
			++next_iteration_length;
		}

		// west
		new_pos = current_pos - 1;
		if (new_pos >= 0 && flags[new_pos] && (edge = EDGE(game->graph, current_pos, new_pos)) >= 1 && edge <= 4) {
			flags[new_pos] = false;
			QUEUE_ADD(queue, n2, queue_start, queue_size, new_pos);
			++next_iteration_length;
		}

		// east
		new_pos = current_pos + 1;
		if (new_pos < n2 && flags[new_pos] && (edge = EDGE(game->graph, current_pos, new_pos)) >= 1 && edge <= 4) {
			flags[new_pos] = false;
			QUEUE_ADD(queue, n2, queue_start, queue_size, new_pos);
			++next_iteration_length;
		}

		--iteration_length;

		if (iteration_length == 0) {
			iteration_length = next_iteration_length;
			next_iteration_length = 0;
			++dist;
		}
	}

	return -1;
}

int evaluate(SimpleGameState *game, int depth) {
	int dist = distance(game, game->pos, true);

	// invalid move (no possible path)
	if (dist == -1) {
		return INVALID_MOVE_SCORE;
	}

	int opponent_dist = distance(game, game->opponent_pos, false);

	// invalid move (no possible path)
	if (opponent_dist == -1) {
		return INVALID_MOVE_SCORE;
	}

	// win
	if (dist == 0) {
		return WIN_SCORE - depth * depth;
	}

	// loose
	if (opponent_dist == 0) {
		return LOOSE_SCORE + depth * depth;
	}

	int score = opponent_dist * opponent_dist - dist * dist;

	if (game->pos != -1) {
		score -= abs(game->pos%n - n/2);
	}

	if (game->opponent_pos != -1) {
		score += abs(game->opponent_pos%n - n/2);
	}

	return score;
}

void invert_int(int *a, int *b) {
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

void invert_ptr(int **a, int **b) {
	int *tmp = *a;
	*a = *b;
	*b = tmp;
}

void change_side(SimpleGameState *game) {
	invert_int(&game->pos, &game->opponent_pos);
	invert_int(&game->num_walls, &game->opponent_num_walls);
	invert_ptr(&game->start_pos, &game->opponent_start_pos);
	game->target_is_up = !game->target_is_up;
}

void apply_move(SimpleGameState *state, unsigned move) {
	enum movetype_t move_type = move >> 24;
	int embed_int = (int) (move & 0x8000 ? move | 0xFFFF0000 : move & 0xFFFF);
	bool embed_bool = move >> 16 & 0xFF;

	switch (move_type) {
		case MOVE:
			state->pos += embed_int;
			break;

		case WALL:
			--(state->num_walls);

			// get nodes
			int first_node = embed_int;
			int second_node = first_node + (embed_bool ? n : 1);

			if (first_node + 1 == second_node) {
				// vertical wall
				EDGE(state->graph, first_node, second_node) = 5;
				EDGE(state->graph, second_node, first_node) = 5;

				EDGE(state->graph, first_node + n, second_node + n) = 6;
				EDGE(state->graph, second_node + n, first_node + n) = 6;
			} else {
				// horizontal wall
				EDGE(state->graph, first_node, second_node) = 7;
				EDGE(state->graph, second_node, first_node) = 7;

				EDGE(state->graph, first_node + 1, second_node + 1) = 8;
				EDGE(state->graph, second_node + 1, first_node + 1) = 8;
			}

			break;

		default:
			break;
	}

	change_side(state);
}

void undo_move(SimpleGameState *state, unsigned move) {
	change_side(state);

	enum movetype_t move_type = move >> 24;
	int embed_int = (int) (move & 0x8000 ? move | 0xFFFF0000 : move & 0xFFFF);
	bool embed_bool = move >> 16 & 0xFF;

	switch (move_type) {
		case MOVE:
			state->pos -= embed_int;
			break;

		case WALL:
			++(state->num_walls);

			// get nodes
			int first_node = (short) (move & 0xFF);
			int second_node = first_node + (embed_bool ? n : 1);

			if (first_node + 1 == second_node) {
				// vertical wall
				EDGE(state->graph, first_node, second_node) = 4;
				EDGE(state->graph, second_node, first_node) = 3;

				EDGE(state->graph, first_node + n, second_node + n) = 4;
				EDGE(state->graph, second_node + n, first_node + n) = 3;
			} else {
				// horizontal wall
				EDGE(state->graph, first_node, second_node) = 2;
				EDGE(state->graph, second_node, first_node) = 1;

				EDGE(state->graph, first_node + 1, second_node + 1) = 2;
				EDGE(state->graph, second_node + 1, first_node + 1) = 1;
			}

			break;

		default:
			break;
	}
}

bool is_game_terminated(SimpleGameState *game) {
	if (game->pos == -1 || game->opponent_pos == -1) {
		return false;
	}

	if (game->target_is_up) {
		return game->pos < n || game->opponent_pos >= n2 - n;
	} else {
		return game->opponent_pos < n || game->pos >= n2 - n;
	}
}

long get_time() {
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC_RAW, &time);
	return time.tv_sec * 1000 + time.tv_nsec / 1000000;
}

int alpha_beta(SimpleGameState *game, int current_depth, int final_depth, int alpha, int beta, unsigned *best_move, long time_limit, bool *aborted) {
	if (*aborted || (final_depth > 1 && current_depth == 1 && get_time() > time_limit)) {
		*aborted = true;
		return 0;
	}

	if (current_depth == final_depth || is_game_terminated(game)) {
		return evaluate(game, current_depth);
	}

	int nb_of_moves;
	unsigned *moves = get_possible_moves(game, &nb_of_moves);

	for (int i = 0; i < nb_of_moves; ++i) {
		unsigned move = moves[i];

		apply_move(game, move);
		int score = -alpha_beta(game, current_depth + 1, final_depth, -beta, -alpha, NULL, time_limit, aborted);
		undo_move(game, move);

		if (score == -INVALID_MOVE_SCORE) {
			continue;
		}

		if (score >= beta) {
			free(moves);
			return beta;
		}

		if (score > alpha) {
			alpha = score;
			if (best_move) {
				*best_move = move;
			}
		}
	}

	free(moves);
	return alpha;
}

unsigned search_best_move(SimpleGameState *game) {
	unsigned best_move = 0;

	int depth = 1;
	long max_time = get_time() + TOTAL_TIME_AVAILABLE / AVG_NB_OF_TURN;

	while (true) {
		bool aborted = false;
		unsigned best_move_for_current_depth;
		alpha_beta(game, 0, depth, -SCORE_LIMIT, SCORE_LIMIT, &best_move_for_current_depth, max_time, &aborted);

		if (aborted) {
			printf("(reached depth: %d)", depth - 1);
			break;
		}

		best_move = best_move_for_current_depth;
		++depth;
	}

	return best_move;
}

SimpleGameState compress_game(struct game_state_t game) {
	SimpleGameState compressed = {
		.graph = malloc(n4),
		.pos = game.self.pos == SIZE_MAX ? -1 : (int) game.self.pos,
		.num_walls = (int) game.self.num_walls,
		.opponent_pos = game.opponent.pos == SIZE_MAX ? -1 : (int) game.opponent.pos,
		.opponent_num_walls = (int) game.opponent.num_walls,
		.start_pos = start_pos,
		.opponent_start_pos = opponent_start_pos,
		.target_is_up = target_is_up
	};

	for (int i = 0; i < n2; ++i) {
		for (int j = 0; j < n2; ++j) {
			EDGE(compressed.graph, i, j) = (char) gsl_spmatrix_uint_get(game.graph->t, i, j);
		}
	}

	return compressed;
}

struct move_t expand_move(struct game_state_t game, unsigned move) {
	enum movetype_t move_type = move >> 24;
	int embed_int = (int) (move & 0x8000 ? move | 0xFFFF0000 : move & 0xFFFF);
	bool embed_bool = move >> 16 & 0xFF;

	struct move_t expanded = {
			.c = game.self.color,
			.t = move_type,
			.m = move_type == MOVE ? (game.self.pos + embed_int) : game.self.pos
	};

	if (move_type == WALL) {
		if (embed_bool) {
			expanded.e[0] = (struct edge_t) {embed_int, embed_int + n};
			expanded.e[1] = (struct edge_t) {embed_int + 1, embed_int + 1 + n};
		} else {
			expanded.e[0] = (struct edge_t) {embed_int, embed_int + 1};
			expanded.e[1] = (struct edge_t) {embed_int + n, embed_int + n + 1};
		}

	} else {
		expanded.e[0] = no_edge();
		expanded.e[1] = no_edge();
	}

	return expanded;
}

struct move_t make_move(struct game_state_t game) {
	SimpleGameState compressed_game = compress_game(game);
	unsigned best_move = search_best_move(&compressed_game);
	free(compressed_game.graph);
	return expand_move(game, best_move);
}

void init_meta(struct game_state_t state) {
	n  = (int) sqrtl(state.graph->t->size1);
	n2 = (int) state.graph->t->size1;
	n4 = n2 * n2;

	start_pos = malloc(sizeof(int) * n);
	nb_of_start_pos = 0;

	opponent_start_pos = malloc(sizeof(int) * n);
	int nb_of_opponent_start_pos = 0;

	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < n; ++j) {
			int pos = j;
			if (i) pos += n2 - n;

			if (gsl_spmatrix_uint_get(state.graph->o, state.self.color, pos)) {
				start_pos[nb_of_start_pos++] = pos;
			} else if (gsl_spmatrix_uint_get(state.graph->o, state.opponent.color, pos)) {
				opponent_start_pos[nb_of_opponent_start_pos++] = pos;
			}
		}
	}

	if (nb_of_start_pos != nb_of_opponent_start_pos) {
		exit(EXIT_FAILURE);
	}

	if (nb_of_start_pos < n) {
		start_pos = realloc(start_pos, sizeof(int) * nb_of_start_pos);
		opponent_start_pos = realloc(opponent_start_pos, sizeof(int) * nb_of_start_pos);
	}

	target_is_up = start_pos[0] >= n;
}

struct move_t make_first_move(struct game_state_t game) {
	init_meta(game);
	return make_move(game);
}

void finalize_ia() {
	free(start_pos);
	free(opponent_start_pos);
}
