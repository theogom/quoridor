/**
 * @file board.h
 *
 * @brief Board interface
 */

#ifndef _QUOR_BOARD_H_
#define _QUOR_BOARD_H_

#include "opt.h"
#include "graph.h"
#include <stdbool.h>


#define IMPOSSIBLE_DISTANCE 500000


/** @enum Move direction */
enum direction_t {
	NO_DIRECTION, NORTH, SOUTH, WEST, EAST, MAX_DIRECTION
};

/** @enum Wall orientation */
enum orientation_t {
	HORIZONTAL, VERTICAL, ERROR_ORIENTATION = -1
};

/** @brief A special vertex used to specify that there is no vertex */
static inline size_t no_vertex(void) {
	return SIZE_MAX;
}

/** @brief A matcher for the special vertex */
static inline bool is_no_vertex(const size_t v) {
	return v == no_vertex();
}

/** @brief Get the adjacent vertex of a vertex in a given direction */
size_t vertex_from_direction(const struct graph_t* graph, size_t v, enum direction_t d);

/** @brief Check if vertex src is linked to vertex dest (i.e there is an edge between them) */
bool is_linked(const struct graph_t* graph, size_t src, size_t dest);

/** @brief Get all the linked vertices from the vertex v */
size_t get_linked(const struct graph_t* graph, size_t v, size_t vertices[]);

/** @brief Initialize a graph representing the game board with the given size and shape */
struct graph_t* graph_init(size_t n, enum shape_t shape);

/** @brief Free the memory allocated for a graph */
void graph_free(struct graph_t* graph);

/** @brief Add edges to graph */
void place_wall(struct graph_t* graph, struct edge_t e[2]);

/** @brief Remove edges from graph */
void remove_wall(struct graph_t* graph, struct edge_t e[2]);

/** @brief Get the opposite to a direction */
enum direction_t opposite(enum direction_t d);

void display_board(struct graph_t* board, size_t board_size, size_t position_player_1, size_t position_player_2);

void display_adj_matrix(struct graph_t* board, size_t board_size);

/** @brief Dijkstra algorithm to get the closest path */
size_t dijkstra(struct graph_t *graph, size_t pos, enum color_t color);

#endif // _QUOR_BOARD_H_
