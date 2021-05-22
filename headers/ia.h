/**
 * @file ia.h
 *
 * @brief IA interface
 */

#ifndef _QUOR_IA_H_
#define _QUOR_IA_H_

#include "graph.h"
#include "move.h"

/** @struct Struct representing the current player state */
struct player_state_t {
	enum color_t color; /**< Player color */
	size_t pos; 		/**< Current position */
	size_t num_walls; 	/**< Number of walls remaining */
};

/** @struct Struct representing a game state */
struct game_state_t {
	struct graph_t* graph; 			/**< Graph representing the game board */
	struct player_state_t self; 	/**< State of the current player */
	struct player_state_t opponent; /**< State of the opponent */
};

/** @brief Return a first move based on the IA strategy */
struct move_t make_first_move(struct game_state_t game);

/** @brief Return a move based on the IA strategy */
struct move_t make_move(struct game_state_t game);

/** @brief Function called when a game is finished */
void finalize_ia();

#endif // _QUOR_IA_H_
