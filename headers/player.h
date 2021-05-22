/**
 * @file player.h
 *
 * @brief Player interface
 */

#ifndef _QUOR_PLAYER_H_
#define _QUOR_PLAYER_H_

#include "graph.h"
#include "move.h"

/** @brief Access to player information */
char const* get_player_name(void);

/** @brief Initialize the player */
void initialize(enum color_t id, struct graph_t* graph, size_t num_walls);

/** @brief Computes next move */
struct move_t play(struct move_t previous_move);

/** @brief Clean up the memory using by the player */
void finalize(void);

#endif // _QUOR_PLAYER_H_
