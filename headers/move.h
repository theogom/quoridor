/**
 * @file move.h
 *
 * @brief Edges and moves definitions
 */

#ifndef _QUOR_MOVE_H_
#define _QUOR_MOVE_H_

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

 /** @enum Players colors */
enum color_t {
	BLACK = 0, WHITE = 1, NO_COLOR = 2
};

/** @enum Types of moves */
enum movetype_t {
	WALL = 0, 	/**< Put a wall */
	MOVE = 1, 	/**< Move to another vertex */
	NO_TYPE = 2 /**< Type representing the very first move of the game */
};

/** @struct Struct representing an oriented edge between two vertex */
struct edge_t {
	size_t fr, 	/**< Source vertex */
		to; 	/**< Destination vertex */
};

/** @struct Struct representing a game move */
struct move_t {
	size_t m;           /**< Id of a vertex */
	struct edge_t e[2]; /**< Two edges for the wall, set to -1 if not applicable */
	enum movetype_t t;  /**< Type of the move */
	enum color_t c;     /**< Color of the player */
};

/** @brief A special edge used to specify that this edge should not be used */
static inline struct edge_t no_edge(void) {
	return (struct edge_t) { SIZE_MAX, SIZE_MAX };
}

/** @brief A matcher for the special edges */
static inline int is_no_edge(const struct edge_t e) {
	return (e.fr == SIZE_MAX) && (e.to == SIZE_MAX);
}

#endif // _QUOR_MOVE_H_
