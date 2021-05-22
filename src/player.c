#include "player.h"
#include "ia.h"
#include "board.h"

struct game_state_t game;
extern char *name;

/** 
 * @brief Access to player information
 *  
 * @return The player name as an [a-zA-Z0-9 -_]* string
 */
char const *get_player_name(void) {
	return name;
}

/**
 * @brief Initialize the player
 * 
 * @details Preconditions:
 * - `id` is either `BLACK` or `WHITE`
 * - `graph` is a heap-allocated copy of the graph where the game is
 *   played, that must be freed in the end
 * - `num_walls` is the number of edges of `graph` divided by 15,
	 rounded up
 * - `initialize()` has never been called before
 * 
 * @param id The color assigned to the player
 * @param graph The graph where the game is played
 * @param num_walls The number of walls assigned to the player
 */
void initialize(enum color_t id, struct graph_t *graph, size_t num_walls) {
	game.graph = graph;

	game.self = (struct player_state_t) {
			.color = id,
			.pos = SIZE_MAX, // default value (no position for now)
			.num_walls = num_walls
	};

	game.opponent = (struct player_state_t) {
			.color = id == BLACK ? WHITE : BLACK,
			.pos = SIZE_MAX, // default value (no position for now)
			.num_walls = num_walls
	};
}

/** 
 * @brief Update the player graph with the given move
 * 
 * @details Update player position if move is a displacement
 * or place the wall in the current player graph
 * 
 * @param move The last game move 
 */
void update_graph(struct move_t move) {
	struct player_state_t *player = move.c == game.self.color ? &game.self : &game.opponent;

	switch (move.t) {
		case MOVE:
			player->pos = move.m;
			break;

		case WALL:
			place_wall(game.graph, move.e);
			--player->num_walls;
			break;

		case NO_TYPE:
			break;
	}
}

/**
 * @brief Print information of a move
 * 
 * @param move The move to print
 */
void print_move(struct move_t move) {
	printf("\n\nMove from %u (%s):\n", move.c, get_player_name());
	printf("vertex: %zu\n", move.m);
	printf("walls: ");

	if (is_no_edge(move.e[0])) {
		printf("(no edge)");
	} else {
		printf("(%zu, %zu)", move.e[0].fr, move.e[0].to);
	}

	printf(" ");

	if (is_no_edge(move.e[1])) {
		printf("(no edge)");
	} else {
		printf("(%zu, %zu)", move.e[1].fr, move.e[1].to);
	}

	printf("\n");
	printf("move type: %s\n\n\n", move.t == 0 ? "WALL" : "MOVE");
}

/** 
 * @brief Computes next move
 *
 * @param previous_move The move from the previous player
 * 
 * @return The next move for the player
 */
struct move_t play(struct move_t previous_move) {
	update_graph(previous_move);

	static bool first_move = true;
	struct move_t move;
	if (first_move) {
		move = make_first_move(game);
		first_move = false;
	} else {
		move = make_move(game);
	}
	// print_move(move);

	update_graph(move);

	return move;
}

/**
 * @brief Clean up the memory using by the player
 * 
 * @details Every allocation done during the calls to initialize and play
 * functions will be freed
 */
void finalize() {
	finalize_ia();
	graph_free(game.graph);
}
